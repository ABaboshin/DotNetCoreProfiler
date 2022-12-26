#include <algorithm>
#include "cor.h"
#include "corprof.h"
#include <corhlpr.h>
#include "MethodRewriter.h"
#include "logging/logging.h"
#include "CorProfiler.h"
#include "util/helpers.h"
#include "const/const.h"

/*

The idea behind is to rewrite the original method in the following way

Exception methodException = null;
try {
    try {
        Interceptor.Before(method params);
    } catch()
    {
    // ignore interceptor's exceptions
    }

    .... original method body ...
} catch (Exception e) {
    methodException = e;
} finally {
    try {
        Interceptor.After(result if not void otherwise null, methodException);
    } catch()
    {
    // ignore interceptor's exceptions
    }
}

*/

HRESULT MethodRewriter::RewriteTargetMethod(ModuleID moduleId, mdMethodDef methodId, ICorProfilerFunctionControl* pFunctionControl)
{
    HRESULT hr;
    mdToken functionToken;
    ClassID classId;

    const auto interceptor = std::find_if(profiler->rejitInfo.begin(), profiler->rejitInfo.end(), [moduleId, methodId](const RejitInfo& ri) {
        return ri.methodId == methodId && ri.moduleId == moduleId;
    });

    // no rejit info => exit
    if (interceptor == profiler->rejitInfo.end()) return S_OK;

    logging::log(logging::LogLevel::INFO, "GetReJITParameters {0}.{1}"_W, interceptor->info.Type.Name, interceptor->info.Name);

    auto rewriter = profiler->CreateILRewriter(pFunctionControl, moduleId, methodId);
    hr = rewriter->Import();
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "GetReJITParameters rewriter->Import failed"_W);
        return hr;
    }

    rewriter::ILRewriterHelper helper(rewriter);
    helper.SetILPosition(rewriter->GetILList()->m_pNext);

    if (profiler->loadedModules.find(interceptor->interceptor.Interceptor.AssemblyName) == profiler->loadedModules.end())
    {
        logging::log(logging::LogLevel::VERBOSE, "Skip rejit because interceptor assembly is not loaded yet"_W);
        return hr;
    }
    else
    {
        logging::log(logging::LogLevel::VERBOSE, "Continue rejit as interceptor assembly is loaded {0}"_W, profiler->loadedModules[interceptor->interceptor.Interceptor.AssemblyName]);
    }

    ComPtr<IUnknown> metadataInterfaces;
    hr = profiler->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf());
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed GetModuleMetaData {0}"_W, interceptor->interceptor.Interceptor.AssemblyName);
        return hr;
    }

    auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);
    auto metadataAssemblyEmit = metadataInterfaces.As<IMetaDataAssemblyEmit>(IID_IMetaDataAssemblyEmit);
    auto assemblyImport = metadataInterfaces.As<IMetaDataAssemblyImport>(IID_IMetaDataEmit);
    auto metadataEmit = metadataInterfaces.As<IMetaDataEmit2>(IID_IMetaDataEmit);

    logging::log(logging::LogLevel::VERBOSE, "{0}"_W, profiler->ilDumper.DumpILCodes("rejit before ", rewriter, interceptor->info, metadataImport));

    // define mscorlib.dll
    mdModuleRef mscorlibRef;
    hr = profiler->GetOrAddAssemblyRef(moduleId, _const::mscorlib, mscorlibRef);
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed GetOrAddAssemblyRef {0}"_W, _const::mscorlib);
        return hr;
    }

    // Define System.Exception
    mdTypeRef exceptionTypeRef;
    hr = profiler->GetOrAddTypeRef(moduleId,mscorlibRef, _const::SystemException.data(), exceptionTypeRef);
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed GetOrAddTypeRef {0}"_W, _const::SystemException);
        return hr;
    }

    // define interceptor.dll
    mdModuleRef baseDllRef;
    hr = profiler->GetOrAddAssemblyRef(moduleId, interceptor->interceptor.Interceptor.AssemblyName, baseDllRef);
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed GetOrAddAssemblyRef {0}"_W, interceptor->interceptor.Interceptor.AssemblyName);
        return hr;
    }

    ULONG exceptionIndex = 0;
    ULONG returnIndex = 0;

    hr = DefineLocalSignature(rewriter, moduleId, exceptionTypeRef, *interceptor, &exceptionIndex, &returnIndex);
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed DefineLocalSignature {0}"_W, interceptor->interceptor.Interceptor.AssemblyName);
        return hr;
    }

    // initialize local variables
    hr = InitLocalVariables(helper, rewriter, metadataEmit, metadataAssemblyEmit, moduleId, *interceptor, exceptionIndex, returnIndex);
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed InitLocalValues {0}"_W, interceptor->interceptor.Interceptor.AssemblyName);
        return hr;
    }

    // define interceptor type
    mdTypeRef interceptorTypeRef;
    hr = profiler->GetOrAddTypeRef(moduleId, baseDllRef, interceptor->interceptor.Interceptor.TypeName.data(), interceptorTypeRef);
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed GetOrAddTypeRef {0}"_W, interceptor->interceptor.Interceptor.TypeName);
        return hr;
    }

    interceptor->info.Signature.ParseArguments();

    // call Before method
    rewriter::ILInstr* beginFirst;
    hr = CreateBeforeMethod(helper, &beginFirst, metadataEmit, metadataAssemblyEmit, interceptorTypeRef, *interceptor, moduleId);
    if (FAILED(hr)) {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed CreateBeforeMethod");
        return hr;
    }

    auto leaveBeforeTry = helper.LeaveS();
    rewriter::ILInstr* beforeCatch;
    hr = LogInterceptorException(helper, rewriter, &beforeCatch, metadataEmit, metadataAssemblyEmit, exceptionTypeRef, moduleId);
    if (FAILED(hr)) {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed LogInterceptorException");
        return hr;
    }
    auto leaveBeforeCatch = helper.LeaveS();

    // try/catch for Interceptor.Before
    rewriter::EHClause beforeEx{};
    beforeEx.m_Flags = COR_ILEXCEPTION_CLAUSE_NONE;
    beforeEx.m_pTryBegin = beginFirst;
    beforeEx.m_pTryEnd = beforeCatch;
    beforeEx.m_pHandlerBegin = beforeCatch;
    beforeEx.m_pHandlerEnd = leaveBeforeCatch;
    beforeEx.m_ClassToken = exceptionTypeRef;

    leaveBeforeCatch->m_pTarget = helper.GetCurrentInstr();
    leaveBeforeTry->m_pTarget = helper.GetCurrentInstr();

    // new ret instruction
    auto newRet = helper.NewInstr();
    newRet->m_opcode = rewriter::CEE_RET;
    rewriter->InsertAfter(rewriter->GetILList()->m_pPrev, newRet);
    helper.SetILPosition(newRet);

    // main catch
    auto mainCatch = helper.StLocal(exceptionIndex);
    auto mainCatchLeave = helper.Rethrow(); // helper.LeaveS();
    mainCatchLeave->m_pTarget = newRet;
    helper.SetILPosition(newRet);
    //auto rethrow = helper.StLocal(exceptionIndex);
    auto nopFinally = helper.Nop();

    // finally

    // call After method
    rewriter::ILInstr* afterFirst;
    hr = CreateAfterMethod(helper, &afterFirst, metadataEmit, metadataAssemblyEmit, interceptorTypeRef, *interceptor, returnIndex, exceptionTypeRef, exceptionIndex, moduleId);
    if (FAILED(hr)) {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed CreateAfterMethod");
        return hr;
    }

    auto leaveAfterTry = helper.LeaveS();
    // TODO log exception
    //auto nopAfterCatch = helper.Pop();
    rewriter::ILInstr* nopAfterCatch;
    hr = LogInterceptorException(helper, rewriter, &nopAfterCatch, metadataEmit, metadataAssemblyEmit, exceptionTypeRef, moduleId);
    if (FAILED(hr)) {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed LogInterceptorException");
        return hr;
    }
    auto leaveAfterCatch = helper.LeaveS();

    // try/catch for Interceptor.After
    rewriter::EHClause afterEx{};
    afterEx.m_Flags = COR_ILEXCEPTION_CLAUSE_NONE;
    afterEx.m_pTryBegin = afterFirst;
    afterEx.m_pTryEnd = nopAfterCatch;
    afterEx.m_pHandlerBegin = nopAfterCatch;
    afterEx.m_pHandlerEnd = leaveAfterCatch;
    afterEx.m_ClassToken = exceptionTypeRef;

    // finally

    // if ex != null throw ex
    /*auto ldLocal = helper.LoadLocal(exceptionIndex);
    auto brFalseS = helper.BrFalseS();
    helper.LoadLocal(exceptionIndex);
    helper.Throw();*/

    auto endFinally = helper.EndFinally();
    leaveAfterCatch->m_pTarget = endFinally;// ldLocal;
    leaveAfterTry->m_pTarget = endFinally;
    //brFalseS->m_pTarget = endFinally;

    auto isVoid = interceptor->info.Signature.ReturnType.IsVoid;

    if (!isVoid) {
        helper.LoadLocal(returnIndex);
    }

    // ret -> leave_s
    for (auto pInstr = rewriter->GetILList()->m_pNext; pInstr != rewriter->GetILList(); pInstr = pInstr->m_pNext)
    {
        switch (pInstr->m_opcode)
        {
        case rewriter::CEE_RET:
        {
            if (pInstr != newRet)
            {
                if (isVoid)
                {
                    pInstr->m_opcode = rewriter::CEE_LEAVE_S;
                    pInstr->m_pTarget = endFinally->m_pNext;
                }
                else
                {
                    pInstr->m_opcode = rewriter::CEE_STLOC;
                    pInstr->m_Arg16 = static_cast<INT16>(returnIndex);

                    auto leaveInstr = rewriter->NewILInstr();
                    leaveInstr->m_opcode = rewriter::CEE_LEAVE_S;
                    leaveInstr->m_pTarget = endFinally->m_pNext;
                    rewriter->InsertAfter(pInstr, leaveInstr);
                }
            }
            break;
        }
        default:
            break;
        }
    }

    rewriter::EHClause exClause{};
    exClause.m_Flags = COR_ILEXCEPTION_CLAUSE_NONE;
    exClause.m_pTryBegin = beginFirst;
    exClause.m_pTryEnd = mainCatch;
    exClause.m_pHandlerBegin = mainCatch;
    exClause.m_pHandlerEnd = mainCatchLeave;
    exClause.m_ClassToken = exceptionTypeRef;

    rewriter::EHClause finallyClause{};
    finallyClause.m_Flags = COR_ILEXCEPTION_CLAUSE_FINALLY;
    finallyClause.m_pTryBegin = beginFirst;
    finallyClause.m_pTryEnd = mainCatchLeave->m_pNext;
    finallyClause.m_pHandlerBegin = nopFinally;// rethrow->m_pNext;
    finallyClause.m_pHandlerEnd = endFinally;

    auto ehCount = rewriter->GetEHCount();
    auto ehPointer = rewriter->GetEHPointer();

    auto newEx = new rewriter::EHClause[ehCount + 4];
    for (unsigned i = 0; i < ehCount; i++)
    {
        newEx[i] = ehPointer[i];
    }

    ehCount += 4;
    newEx[ehCount - 4] = beforeEx;
    newEx[ehCount - 3] = afterEx;
    newEx[ehCount - 2] = exClause;
    newEx[ehCount - 1] = finallyClause;
    rewriter->SetEHClause(newEx, ehCount);

    logging::log(logging::LogLevel::VERBOSE, "{0}"_W, profiler->ilDumper.DumpILCodes("rejit after ", rewriter, interceptor->info, metadataImport));

    hr = rewriter->Export();

    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "GetReJITParameters rewriter->Export failed"_W);
        return hr;
    }

    logging::log(logging::LogLevel::INFO, "GetReJITParameters done"_W);

    return S_OK;
}

HRESULT MethodRewriter::GetTargetTypeRef(const info::TypeInfo& targetType, util::ComPtr< IMetaDataEmit2>& metadataEmit, util::ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdToken* targetTypeRef, bool* isValueType, ModuleID moduleId) {
    HRESULT hr;
    *isValueType = targetType.IsValueType;
    *targetTypeRef = targetType.TypeSpec;

    if (*targetTypeRef != mdTypeSpecNil) {
        return S_OK;
    }

    // TODO cache references to known types
    mdTypeRef objectTypeRef;
    mdModuleRef mscorlibRef;
    hr = profiler->GetOrAddAssemblyRef(moduleId, _const::mscorlib, mscorlibRef);
    hr = profiler->GetOrAddTypeRef(moduleId, mscorlibRef, _const::SystemObject, objectTypeRef);
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed GetTargetTypeRef GetObjectTypeRef"_W);
        return hr;
    }

    if (targetType.ParentTypeInfo != nullptr) {
        bool parentTypeIsGeneric = false;
        auto parentType = targetType.ParentTypeInfo.get();
        while (true) {
            if (parentType->IsGenericClassRef) {
                parentTypeIsGeneric = true;
                break;
            }

            if (parentType->ParentTypeInfo == nullptr) {
                break;
            }

            parentType = parentType->ParentTypeInfo.get();
        }

        // if parent type is generic
        if (parentTypeIsGeneric) {
            // and the target type is a struct => cannot be instrumentialized
            if (targetType.IsValueType) {
                *targetTypeRef = mdTokenNil;
                return S_OK;
            }

            // otherwise use system.object
            *targetTypeRef = objectTypeRef;

            return S_OK;
        }
    }

    auto currentType = &targetType;
    // if the target type is a generic one
    // lookup for the nearest non generic parent
    while (currentType != nullptr && targetType.IsGenericClassRef) {
        currentType = currentType->ParentTypeInfo.get();
    }

    if (currentType == nullptr)
    {
        *targetTypeRef = objectTypeRef;
        return S_OK;
    }

    *isValueType = currentType->IsValueType;
    *targetTypeRef = currentType->Id;

    return S_OK;
}

