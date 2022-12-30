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

    //auto firstIL = helper.GetCurrentInstr();

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

    // define mscorlib.dll
    mdModuleRef mscorlibRef;
    hr = GetMsCorLibRef(metadataAssemblyEmit, mscorlibRef);
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed GetMsCorLibRef {0}"_W, interceptor->interceptor.Interceptor.AssemblyName);
        return hr;
    }

    // Define System.Exception
    mdTypeRef exceptionTypeRef;
    metadataEmit->DefineTypeRefByName(mscorlibRef, _const::SystemException.data(), &exceptionTypeRef);

    // define interceptor.dll
    mdModuleRef baseDllRef;
    hr = GetAssemblyRef(metadataAssemblyEmit, baseDllRef, interceptor->interceptor.Interceptor.AssemblyName);
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed GetWrapperRef {0}"_W, interceptor->interceptor.Interceptor.AssemblyName);
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
    hr = metadataEmit->DefineTypeRefByName(
        baseDllRef,
        interceptor->interceptor.Interceptor.TypeName.data(),
        &interceptorTypeRef);
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed DefineTypeRefByName {0}"_W, interceptor->interceptor.Interceptor.TypeName);
    }

    interceptor->info.Signature.ParseArguments();

    // try/catch for Interceptor.Before
    rewriter::EHClause beforeEx{};
    rewriter::ILInstr* beginFirst;

    hr = CreateTryCatch(
        [this, &helper, &metadataEmit, &metadataAssemblyEmit, interceptorTypeRef, interceptor, &beginFirst](rewriter::ILInstr** tryBegin, rewriter::ILInstr** tryLeave) {
        // call Before method
        auto hr = CreateBeforeMethod(helper, tryBegin, metadataEmit, metadataAssemblyEmit, interceptorTypeRef, *interceptor);
        if (FAILED(hr)) {
            logging::log(logging::LogLevel::NONSUCCESS, "Failed CreateBeforeMethod");
            return hr;
        }

        *tryLeave = helper.LeaveS();

        beginFirst = *tryBegin;

        return S_OK;
    },
        [this, &helper, &metadataEmit, &metadataAssemblyEmit, interceptorTypeRef, interceptor, exceptionTypeRef, &rewriter](rewriter::ILInstr** catchBegin, rewriter::ILInstr** catchLeave) {
        auto hr = LogInterceptorException(helper, rewriter, catchBegin, metadataEmit, metadataAssemblyEmit, exceptionTypeRef);
        if (FAILED(hr)) {
            logging::log(logging::LogLevel::NONSUCCESS, "Failed LogInterceptorException");
            return hr;
        }
        *catchLeave = helper.LeaveS();
        return S_OK;
    }, [&helper](rewriter::ILInstr& tryLeave) {
        tryLeave.m_pTarget = helper.GetCurrentInstr();
        return S_OK;
    }, [&helper](rewriter::ILInstr& leaveBeforeCatch) {
        leaveBeforeCatch.m_pTarget = helper.GetCurrentInstr();
        return S_OK;
    }, exceptionTypeRef, beforeEx);

    if (FAILED(hr)) {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed CreateTryCatch Before");
        return hr;
    }

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
    auto nopFinally = helper.Nop();

    // finally

    // try/catch for Interceptor.After
    rewriter::EHClause afterEx{};
    rewriter::ILInstr* afterFirst;
    rewriter::ILInstr* leaveAfterTry;
    rewriter::ILInstr* nopAfterCatch;
    rewriter::ILInstr* leaveAfterCatch;
    hr = CreateTryCatch(
        [this, &helper, &metadataEmit, &metadataAssemblyEmit, interceptorTypeRef, interceptor, &afterFirst, returnIndex, exceptionTypeRef, exceptionIndex, &leaveAfterTry](rewriter::ILInstr** tryBegin, rewriter::ILInstr** tryLeave) {
        // call After method
    auto hr = CreateAfterMethod(helper, tryBegin, metadataEmit, metadataAssemblyEmit, interceptorTypeRef, *interceptor, returnIndex, exceptionTypeRef, exceptionIndex);
    if (FAILED(hr)) {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed CreateAfterMethod");
        return hr;
    }

    leaveAfterTry = *tryLeave = helper.LeaveS();
    if (FAILED(hr)) {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed CreateAfterMethod");
        return hr;
    }

    afterFirst = *tryBegin;

    return S_OK;
    },
        [this, &helper, &metadataEmit, &metadataAssemblyEmit, interceptorTypeRef, interceptor, exceptionTypeRef, &rewriter, &leaveAfterCatch, &nopAfterCatch](rewriter::ILInstr** catchBegin, rewriter::ILInstr** catchLeave) {
    auto hr = LogInterceptorException(helper, rewriter, catchBegin, metadataEmit, metadataAssemblyEmit, exceptionTypeRef);
    if (FAILED(hr)) {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed LogInterceptorException");
        return hr;
    }

    nopAfterCatch = *catchBegin;
    leaveAfterCatch = *catchLeave = helper.LeaveS();
    return S_OK;
    }, [&helper](rewriter::ILInstr& tryLeave) {
        
    return S_OK;
    }, [&helper](rewriter::ILInstr& leaveBeforeCatch) {
        
    return S_OK;
    }, exceptionTypeRef, afterEx);

    if (FAILED(hr)) {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed CreateTryCatch After");
        return hr;
    }

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

    hr = rewriter->Export();

    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "GetReJITParameters rewriter->Export failed"_W);
        return hr;
    }

    logging::log(logging::LogLevel::INFO, "GetReJITParameters done"_W);

    return S_OK;
}

HRESULT MethodRewriter::GetTargetTypeRef(const info::TypeInfo& targetType, util::ComPtr< IMetaDataEmit2>& metadataEmit, util::ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, mdToken* targetTypeRef, bool* isValueType) {
    HRESULT hr;
    *isValueType = targetType.IsValueType;
    *targetTypeRef = targetType.TypeSpec;

    if (*targetTypeRef != mdTypeSpecNil) {
        return S_OK;
    }

    // TODO cache references to known types
    mdTypeRef objectTypeRef;
    hr = GetObjectTypeRef(metadataEmit, metadataAssemblyEmit, &objectTypeRef);
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

