#include <algorithm>
#include <iostream>
#include <string>
#include "corhlpr.h"
#include "corhdr.h"
#include "configuration/back_inserter.h"
#include "configuration/Configuration.h"
#include "const/const.h"
#include "info/InterceptionVarInfo.h"
#include "info/parser.h"
#include "rewriter/ILRewriterHelper.h"
#include "util/helpers.h"
#include "util/util.h"
#include "util/ComPtr.h"
#include "CorProfiler.h"
#include "dllmain.h"
#include "logging/logging.h"
#include "ILDumper.h"

HRESULT STDMETHODCALLTYPE CorProfiler::JITCompilationStarted(FunctionID functionId, BOOL fIsSafeToBlock)
{
    std::lock_guard<std::mutex> guard(mutex);

    HRESULT hr;
    mdToken functionToken;
    ClassID classId;
    ModuleID moduleId;

    IfFailRet(this->corProfilerInfo->GetFunctionInfo(functionId, &classId, &moduleId, &functionToken));

    auto moduleInfo = info::ModuleInfo::GetModuleInfo(this->corProfilerInfo, moduleId);

    // quick exit if already loaded into app domain
    if (loadedIntoAppDomains.find(moduleInfo.assembly.appDomainId) != loadedIntoAppDomains.end())
    {
        return S_OK;
    }

    // if the current call is not a call to one of skipped assemblies
    if (skippedModules.find(moduleId) != skippedModules.end())
    {
        //logging::log(
        //    logging::LogLevel::VERBOSE,
        //    "Skip intercept assembly {0} in app_domain_id {1}"_W,
        //    moduleInfo.assembly.name,
        //    moduleInfo.assembly.appDomainId);

        return S_OK;
    }

    // white listed
    /*if (std::find(enabledModules.begin(), enabledModules.end(), moduleId) == enabledModules.end())
    {
      return S_OK;
    }*/

    auto rewriter = CreateILRewriter(nullptr, moduleId, functionToken);
    IfFailRet(rewriter->Import());

    // load once into appdomain
    //if (loadedIntoAppDomains.find(moduleInfo.assembly.appDomainId) == loadedIntoAppDomains.end())
    //{
    loadedIntoAppDomains.insert(moduleInfo.assembly.appDomainId);

    ComPtr<IUnknown> metadataInterfaces;
    IfFailRet(this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf()));

    const auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);

    const auto functionInfo = info::FunctionInfo::GetFunctionInfo(metadataImport, functionToken);

    //logging::log(
    //    logging::LogLevel::INFO,
    //    "Load into app_domain_id {0} Before call to {1}.{2} num args {3} from assembly {4}"_W,
    //    moduleInfo.assembly.appDomainId,
    //    functionInfo.Type.Name,
    //    functionInfo.Name,
    //    functionInfo.Signature.NumberOfArguments(),
    //    moduleInfo.assembly.name);

    IfFailRet(InjectLoadMethod(moduleId, *rewriter, functionInfo));

    hr = rewriter->Export();

    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::INFO, "Failed to InjectLoadMethod"_W);
        return S_FALSE;
    }

    //}

    return S_OK;

    // auto result = Rewrite(moduleId, *rewriter, alreadyChanged);

    // delete rewriter;

    // return result;
}

HRESULT CorProfiler::InjectLoadMethod(ModuleID moduleId, rewriter::ILRewriter& rewriter, const info::FunctionInfo& functionInfo)
{
    mdMethodDef retMethodToken;

    auto hr = GenerateLoadMethod(moduleId, retMethodToken, functionInfo);
    IfFailRet(hr);

    rewriter::ILRewriterHelper helper(&rewriter);
    helper.SetILPosition(rewriter.GetILList()->m_pNext);
    helper.CallMember(retMethodToken, false);

    ComPtr<IUnknown> metadataInterfaces;
    IfFailRet(this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf()));
    const auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);

    std::cout << ilDumper.DumpILCodes("main ", &rewriter, functionInfo, metadataImport) << std::endl;

    return S_OK;
}

HRESULT CorProfiler::GenerateLoadMethod(ModuleID moduleId, mdMethodDef& retMethodToken, const info::FunctionInfo& functionInfo)
{
    HRESULT hr;

    ComPtr<IUnknown> metadataInterfaces;
    IfFailRet(this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf()));

    const auto metadataEmit = metadataInterfaces.As<IMetaDataEmit2>(IID_IMetaDataEmit);
    const auto metadataAssemblyEmit = metadataInterfaces.As<IMetaDataAssemblyEmit>(IID_IMetaDataAssemblyEmit);
    const auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);

    // define mscorlib.dll
    mdModuleRef mscorlibRef;
    GetMsCorLibRef(hr, metadataAssemblyEmit, mscorlibRef);
    IfFailRet(hr);

    // Define System.Object
    mdTypeRef objectTypeRef;
    metadataEmit->DefineTypeRefByName(mscorlibRef, _const::SystemObject.data(), &objectTypeRef);

    // Define an anonymous type
    mdTypeDef newTypeDef;
    hr = metadataEmit->DefineTypeDef("__InterceptionDllLoaderClass__"_W.c_str(), tdAbstract | tdSealed,
        objectTypeRef, NULL, &newTypeDef);

    // Define a a new static method
    COR_SIGNATURE loadMethodSignature[] = {
        IMAGE_CEE_CS_CALLCONV_DEFAULT,
        0,
        ELEMENT_TYPE_VOID };

    hr = metadataEmit->DefineMethod(newTypeDef,
        "__InterceptionDllLoaderMethod__"_W.c_str(),
        mdStatic | mdPublic | miAggressiveInlining,
        loadMethodSignature,
        sizeof(loadMethodSignature),
        0,
        0,
        &retMethodToken);

    // System.Reflection.Assembly
    mdTypeRef assemblyTypeRef;
    hr = metadataEmit->DefineTypeRefByName(mscorlibRef, _const::SystemReflectionAssembly.data(), &assemblyTypeRef);

    // Assembly.LoadFrom
    BYTE compressedToken[10];
    ULONG tokenLength = CorSigCompressToken(assemblyTypeRef, compressedToken);

    std::vector<BYTE> assemblyLoadFromSignature = {
        IMAGE_CEE_CS_CALLCONV_DEFAULT,
        1,
        ELEMENT_TYPE_CLASS };
    assemblyLoadFromSignature.insert(assemblyLoadFromSignature.end(), compressedToken, compressedToken + tokenLength);
    assemblyLoadFromSignature.push_back(ELEMENT_TYPE_STRING);

    mdMemberRef assemblyLoadFromRef;
    hr = metadataEmit->DefineMemberRef(
        assemblyTypeRef, _const::LoadFrom.data(),
        assemblyLoadFromSignature.data(),
        assemblyLoadFromSignature.size(),
        &assemblyLoadFromRef);

    // Get Assembly.CreateInstance(string)
    COR_SIGNATURE createInstanceSignature[] = { IMAGE_CEE_CS_CALLCONV_HASTHIS, 1,
                                                          ELEMENT_TYPE_OBJECT,
                                                          ELEMENT_TYPE_STRING };

    mdMemberRef createInstanceMemberRef;
    hr = metadataEmit->DefineMemberRef(assemblyTypeRef, "CreateInstance"_W.c_str(),
        createInstanceSignature, sizeof(createInstanceSignature),
        &createInstanceMemberRef);
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::INFO, "Failed to get CreateInstance in load method"_W);
        return hr;
    }

    auto rewriter = CreateILRewriter(nullptr, moduleId, retMethodToken);
    rewriter->InitializeTiny();

    //mdSignature localSignatureToken;
    //COR_SIGNATURE localSignature[7] = {
    //    IMAGE_CEE_CS_CALLCONV_LOCAL_SIG,
    //    1,
    //    ELEMENT_TYPE_CLASS
    //};
    //CorSigCompressToken(assemblyTypeRef, &localSignature[3]);
    //hr = metadataEmit->GetTokenFromSig(localSignature, sizeof(localSignature), &localSignatureToken);
    //if (FAILED(hr))
    //{
    //    logging::log(logging::LogLevel::INFO, "Failed to create local sig in load method"_W);
    //}

    //rewriter->SetTkLocalVarSig(localSignatureToken);

    rewriter::ILRewriterHelper helper(rewriter);
    helper.SetILPosition(rewriter->GetILList()->m_pNext);

    //for (const auto& el : configuration.Assemblies)
    {
        logging::log(logging::LogLevel::INFO, "Load {0}"_W, configuration.Loader.AssemblyPath);

        // assembly path
        mdString pathToken;
        hr = metadataEmit->DefineUserString(configuration.Loader.AssemblyPath.c_str(), (ULONG)configuration.Loader.AssemblyPath.length(), &pathToken);

        helper.LoadStr(pathToken);
        helper.CallMember(assemblyLoadFromRef, false);
        //helper.Pop();
        //helper.StLocal(0);
        //}

        logging::log(logging::LogLevel::INFO, "Loader {0} from {1}"_W, configuration.Loader.TypeName, configuration.Loader.AssemblyPath);


        mdString loaderToken;
        metadataEmit->DefineUserString(configuration.Loader.TypeName.c_str(), (ULONG)configuration.Loader.TypeName.length(), &loaderToken);

        //helper.LoadLocal(0);
        helper.LoadStr(loaderToken);
        helper.CallMember(createInstanceMemberRef, true);
        helper.Pop();

        //_exit:
        // ret
        helper.Ret();

        std::cout << ilDumper.DumpILCodes("load ", rewriter, functionInfo, metadataImport) << std::endl;

        hr = rewriter->Export();

        if (FAILED(hr))
        {
            logging::log(logging::LogLevel::INFO, "Failed to create load method"_W);
        }

        return hr;
    }
}

    //HRESULT CorProfiler::Rewrite(ModuleID moduleId, rewriter::ILRewriter& rewriter, bool alreadyChanged)
    //{
    //    HRESULT hr;
    //
    //    ComPtr<IUnknown> metadataInterfaces;
    //    IfFailRet(this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf()));
    //
    //    auto metadataEmit = metadataInterfaces.As<IMetaDataEmit2>(IID_IMetaDataEmit);
    //    auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);
    //    auto metadataAssemblyEmit = metadataInterfaces.As<IMetaDataAssemblyEmit>(IID_IMetaDataAssemblyEmit);
    //
    //    auto moduleInfo = info::ModuleInfo::GetModuleInfo(this->corProfilerInfo, moduleId);
    //
    //    for (rewriter::ILInstr* pInstr = rewriter.GetILList()->m_pNext;
    //        pInstr != rewriter.GetILList(); pInstr = pInstr->m_pNext)
    //    {
    //        if (pInstr->m_opcode != rewriter::CEE_CALL && pInstr->m_opcode != rewriter::CEE_CALLVIRT)
    //        {
    //            continue;
    //        }
    //
    //        auto target = info::FunctionInfo::GetFunctionInfo(metadataImport, pInstr->m_Arg32);
    //
    //        auto targetMdToken = pInstr->m_Arg32;
    //
    //        logging::log(
    //            logging::LogLevel::INFO, "Found call to {0}.{1} num args {2} from assembly {3}"_W,
    //            target.Type.Name,
    //            target.Name,
    //            target.Signature.NumberOfArguments(),
    //            moduleInfo.assembly.name);
    //
    //        auto interceptions = FindInterceptions(moduleInfo.assembly.name, target);
    //
    //        if (!interceptions.empty())
    //        {
    //            alreadyChanged = true;
    //
    //            logging::log(
    //                logging::LogLevel::INFO, "Intercept call to {0}.{1} num args {2} from assembly {3}"_W,
    //                target.Type.Name,
    //                target.Name,
    //                target.Signature.NumberOfArguments(),
    //                moduleInfo.assembly.name);
    //
    //            rewriter::ILRewriterHelper helper(&rewriter);
    //            helper.SetILPosition(pInstr);
    //
    //            mdMethodDef interceptorRef;
    //            GenerateInterceptMethod(moduleId, target, interceptions, targetMdToken, interceptorRef);
    //
    //            helper.CallMember(interceptorRef, false);
    //
    //            pInstr->m_opcode = rewriter::CEE_NOP;
    //        }
    //    }
    //
    //    if (alreadyChanged)
    //    {
    //        IfFailRet(rewriter.Export());
    //    }
    //
    //    return S_OK;
    //}
    //
    //HRESULT CorProfiler::GenerateInterceptMethod(ModuleID moduleId, info::FunctionInfo& target, const std::vector<configuration::TypeInfo>& interceptions, mdToken targetMdToken, mdMethodDef& retMethodToken)
    //{
    //    HRESULT hr;
    //
    //    ComPtr<IUnknown> metadataInterfaces;
    //    IfFailRet(this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf()));
    //
    //    auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);
    //    auto metadataEmit = metadataInterfaces.As<IMetaDataEmit2>(IID_IMetaDataEmit);
    //    auto assemblyImport = metadataInterfaces.As<IMetaDataAssemblyImport>(IID_IMetaDataEmit);
    //    auto metadataAssemblyEmit = metadataInterfaces.As<IMetaDataAssemblyEmit>(IID_IMetaDataAssemblyEmit);
    //
    //    // define mscorlib.dll
    //    mdModuleRef mscorlibRef;
    //    GetMsCorLibRef(hr, metadataAssemblyEmit, mscorlibRef);
    //
    //    // define interception.core.dll
    //    mdModuleRef baseDllRef;
    //    GetWrapperRef(hr, metadataAssemblyEmit, baseDllRef, configuration.InterceptorInterface.AssemblyName);
    //
    //    // Define System.Object
    //    mdTypeRef objectTypeRef;
    //    metadataEmit->DefineTypeRefByName(mscorlibRef, _const::SystemObject.data(), &objectTypeRef);
    //
    //    // Define System.Exception
    //    mdTypeRef exceptionTypeRef;
    //    metadataEmit->DefineTypeRefByName(mscorlibRef, _const::SystemException.data(), &exceptionTypeRef);
    //
    //    // Define System.Type
    //    mdTypeRef typeRef;
    //    metadataEmit->DefineTypeRefByName(mscorlibRef, _const::SystemType.data(), &typeRef);
    //
    //    // Define System.Type
    //    mdTypeRef runtimeTypeHandleRef;
    //    metadataEmit->DefineTypeRefByName(mscorlibRef, _const::SystemRuntimeTypeHandle.data(), &runtimeTypeHandleRef);
    //
    //    // Define a static type
    //    mdTypeDef newTypeDef;
    //    auto className = target.Type.Name + target.Name + "Interception"_W;
    //    std::replace(className.begin(), className.end(), '.'_W, '_'_W);
    //    hr = metadataEmit->DefineTypeDef(className.c_str(), tdAbstract | tdSealed,
    //        objectTypeRef, NULL, &newTypeDef);
    //
    //    // create method signature
    //    std::vector<BYTE> signature = {
    //        // call convention for static method
    //        IMAGE_CEE_CS_CALLCONV_DEFAULT,
    //        // number of arguments: original count + this if instance method
    //        (BYTE)(target.Signature.NumberOfArguments() + (target.Signature.IsInstanceMethod() ? 1 : 0)) };
    //    // return type
    //    auto retType = target.ResolveParameterType(target.Signature.ReturnType);
    //    signature.insert(signature.end(), retType.Raw.begin(), retType.Raw.end());
    //
    //    // insert this
    //    if (target.Signature.IsInstanceMethod())
    //    {
    //        signature.push_back(ELEMENT_TYPE_OBJECT);
    //    }
    //
    //    target.Signature.ParseArguments();
    //
    //    // insert existing arguments
    //    for (size_t i = 0; i < target.Signature.NumberOfArguments(); i++)
    //    {
    //        auto argument = target.ResolveParameterType(target.Signature.Arguments[i]);
    //        if (argument.IsRefType)
    //        {
    //            signature.push_back(ELEMENT_TYPE_BYREF);
    //        }
    //
    //        signature.push_back(argument.TypeDef);
    //    }
    //
    //    // define a method
    //    hr = metadataEmit->DefineMethod(newTypeDef,
    //        "__InterceptionMethod__"_W.c_str(),
    //        mdStatic | mdPublic | miAggressiveInlining,
    //        signature.data(),
    //        signature.size(),
    //        0,
    //        0,
    //        &retMethodToken);
    //
    //    rewriter::ILRewriter rewriter(this->corProfilerInfo, nullptr, moduleId, retMethodToken);
    //    rewriter::ILRewriterHelper helper(&rewriter);
    //
    //    mdSignature localSigToken;
    //    std::vector<BYTE> localSig = {
    //        IMAGE_CEE_CS_CALLCONV_LOCAL_SIG,
    //        1,
    //        ELEMENT_TYPE_OBJECT,
    //    };
    //
    //    int resultIndex = 0;
    //
    //    hr = metadataEmit->GetTokenFromSig(localSig.data(), localSig.size(), &localSigToken);
    //
    //    rewriter.InitializeTiny();
    //    rewriter.SetTkLocalVarSig(localSigToken);
    //    helper.SetILPosition(rewriter.GetILList()->m_pNext);
    //
    //    // define interceptorInterfaceRef = IInterceptor
    //    mdTypeRef interceptorInterfaceRef;
    //    hr = metadataEmit->DefineTypeRefByName(
    //        baseDllRef,
    //        configuration.InterceptorInterface.TypeName.data(),
    //        &interceptorInterfaceRef);
    //    IfFailRet(hr);
    //
    //    // define composedInterceptorTypeRef = ComposedInterceptor
    //    mdTypeRef composedInterceptorTypeRef;
    //    hr = metadataEmit->DefineTypeRefByName(
    //        baseDllRef,
    //        configuration.ComposedInterceptor.TypeName.data(),
    //        &composedInterceptorTypeRef);
    //    IfFailRet(hr);
    //
    //    // define MethodFinderInterfaceRef = IMethodFinder
    //    mdTypeRef methodFinderInterfaceRef;
    //    hr = metadataEmit->DefineTypeRefByName(
    //        baseDllRef,
    //        configuration.MethodFinderInterface.TypeName.data(),
    //        &methodFinderInterfaceRef);
    //    IfFailRet(hr);
    //
    //    // local sig
    //    std::vector<info::InterceptionVarInfo> inteceptionRefs{};
    //    for (const auto& interception : interceptions)
    //    {
    //        // define interception.dll
    //        mdModuleRef interceptorDllRef;
    //        GetWrapperRef(hr, metadataAssemblyEmit, interceptorDllRef, interception.AssemblyName);
    //
    //        // define wrappedType
    //        mdTypeRef interceptorTypeRef;
    //        hr = metadataEmit->DefineTypeRefByName(
    //            interceptorDllRef,
    //            interception.TypeName.data(),
    //            &interceptorTypeRef);
    //        IfFailRet(hr);
    //
    //        int locaVarIndex = 0;
    //        helper.AddLocalVariable(interceptorTypeRef, locaVarIndex);
    //        inteceptionRefs.push_back(info::InterceptionVarInfo(interceptorTypeRef, locaVarIndex));
    //    }
    //
    //    int composedIndex = 0;
    //    helper.AddLocalVariable(composedInterceptorTypeRef, composedIndex);
    //
    //    int typeIndex = 0;
    //    helper.AddLocalVariable(typeRef, typeIndex);
    //
    //    int methodFinderIndex = 0;
    //    auto methodFinder = FindMethodFinder(target);
    //    mdTypeRef methodFinderTypeRef;
    //    if (std::get<1>(methodFinder))
    //    {
    //        logging::log(
    //            logging::LogLevel::DEBUG, "MethodFinder {0}"_W, std::get<0>(methodFinder).AssemblyName);
    //
    //        // define interception.dll
    //        mdModuleRef methodFinderDllRef;
    //        GetWrapperRef(hr, metadataAssemblyEmit, methodFinderDllRef, std::get<0>(methodFinder).AssemblyName);
    //
    //        hr = metadataEmit->DefineTypeRefByName(
    //            methodFinderDllRef,
    //            std::get<0>(methodFinder).TypeName.data(),
    //            &methodFinderTypeRef);
    //        IfFailRet(hr);
    //
    //        helper.AddLocalVariable(methodFinderTypeRef, methodFinderIndex);
    //    }
    //
    //    // ctor
    //    for (const auto& interceptor : inteceptionRefs)
    //    {
    //        std::vector<BYTE> ctorSignature = {
    //            IMAGE_CEE_CS_CALLCONV_HASTHIS,
    //            0,
    //            ELEMENT_TYPE_VOID };
    //
    //        mdMemberRef ctorRef;
    //        hr = metadataEmit->DefineMemberRef(
    //            interceptor.TypeRef,
    //            _const::ctor.data(),
    //            ctorSignature.data(),
    //            ctorSignature.size(),
    //            &ctorRef);
    //
    //        helper.NewObject(ctorRef);
    //        helper.StLocal(interceptor.LocalVarIndex);
    //    }
    //
    //    if (std::get<1>(methodFinder))
    //    {
    //        logging::log(
    //            logging::LogLevel::DEBUG, "Create MethodFinder {0}"_W, std::get<0>(methodFinder).TypeName);
    //
    //        std::vector<BYTE> ctorSignature = {
    //            IMAGE_CEE_CS_CALLCONV_HASTHIS,
    //            0,
    //            ELEMENT_TYPE_VOID };
    //
    //        mdMemberRef ctorRef;
    //        hr = metadataEmit->DefineMemberRef(
    //            methodFinderTypeRef,
    //            _const::ctor.data(),
    //            ctorSignature.data(),
    //            ctorSignature.size(),
    //            &ctorRef);
    //
    //        helper.NewObject(ctorRef);
    //        helper.StLocal(methodFinderIndex);
    //    }
    //
    //    {
    //        std::vector<BYTE> ctorSignature = {
    //            IMAGE_CEE_CS_CALLCONV_HASTHIS,
    //            0,
    //            ELEMENT_TYPE_VOID };
    //
    //        mdMemberRef ctorRef;
    //        hr = metadataEmit->DefineMemberRef(
    //            composedInterceptorTypeRef,
    //            _const::ctor.data(),
    //            ctorSignature.data(),
    //            ctorSignature.size(),
    //            &ctorRef);
    //
    //        helper.NewObject(ctorRef);
    //        helper.StLocal(composedIndex);
    //    }
    //
    //    auto shift = 0;
    //
    //    //SetThis
    //    if (target.Signature.IsInstanceMethod())
    //    {
    //        std::vector<BYTE> setThisSignature = {
    //            IMAGE_CEE_CS_CALLCONV_HASTHIS,
    //            1,
    //            ELEMENT_TYPE_VOID,
    //            ELEMENT_TYPE_OBJECT };
    //
    //        mdMemberRef setThisRef;
    //        hr = metadataEmit->DefineMemberRef(
    //            composedInterceptorTypeRef,
    //            "set_This"_W.data(),
    //            setThisSignature.data(),
    //            setThisSignature.size(),
    //            &setThisRef);
    //
    //        shift = 1;
    //        helper.LoadLocal(composedIndex);
    //        helper.LoadArgument(0);
    //        helper.CallMember(setThisRef, false);
    //    }
    //
    //    //SetMdToken
    //    {
    //        std::vector<BYTE> setMdTokenSignature = {
    //            IMAGE_CEE_CS_CALLCONV_HASTHIS,
    //            1,
    //            ELEMENT_TYPE_VOID,
    //            ELEMENT_TYPE_I4 };
    //
    //        mdMemberRef setMdTokenRef;
    //        hr = metadataEmit->DefineMemberRef(
    //            composedInterceptorTypeRef,
    //            "set_MdToken"_W.data(),
    //            setMdTokenSignature.data(),
    //            setMdTokenSignature.size(),
    //            &setMdTokenRef);
    //
    //        helper.LoadLocal(composedIndex);
    //        helper.LoadInt32(targetMdToken);
    //        helper.CallMember(setMdTokenRef, false);
    //    }
    //
    //    //MethodName
    //    {
    //        std::vector<BYTE> setMethodNameSignature = {
    //            IMAGE_CEE_CS_CALLCONV_HASTHIS,
    //            1,
    //            ELEMENT_TYPE_VOID,
    //            ELEMENT_TYPE_STRING };
    //
    //        mdMemberRef setMethodNameRef;
    //        hr = metadataEmit->DefineMemberRef(
    //            composedInterceptorTypeRef,
    //            "set_MethodName"_W.data(),
    //            setMethodNameSignature.data(),
    //            setMethodNameSignature.size(),
    //            &setMethodNameRef);
    //
    //        // assembly path
    //        mdString methodNameToken;
    //        hr = metadataEmit->DefineUserString(target.Name.c_str(), (ULONG)target.Name.length(), &methodNameToken);
    //
    //        helper.LoadLocal(composedIndex);
    //        helper.LoadStr(methodNameToken);
    //        helper.CallMember(setMethodNameRef, false);
    //    }
    //
    //    //SetModuleVersionPtr
    //    {
    //        std::vector<BYTE> setModuleVersionPtrSignature = {
    //            IMAGE_CEE_CS_CALLCONV_HASTHIS,
    //            1,
    //            ELEMENT_TYPE_VOID,
    //            ELEMENT_TYPE_I8 };
    //
    //        mdMemberRef setModuleVersionPtrRef;
    //        hr = metadataEmit->DefineMemberRef(
    //            composedInterceptorTypeRef,
    //            "set_ModuleVersionPtr"_W.data(),
    //            setModuleVersionPtrSignature.data(),
    //            setModuleVersionPtrSignature.size(),
    //            &setModuleVersionPtrRef);
    //
    //        helper.LoadLocal(composedIndex);
    //        const void* module_version_id_ptr = &modules[moduleId];
    //        helper.LoadInt64(reinterpret_cast<INT64>(module_version_id_ptr));
    //        helper.CallMember(setModuleVersionPtrRef, false);
    //    }
    //
    //    //SetArgumentCount
    //    {
    //        std::vector<BYTE> setArgumentCountSignature = {
    //            IMAGE_CEE_CS_CALLCONV_HASTHIS,
    //            1,
    //            ELEMENT_TYPE_VOID,
    //            ELEMENT_TYPE_I4 };
    //
    //        mdMemberRef setArgumentCountRef;
    //        hr = metadataEmit->DefineMemberRef(
    //            composedInterceptorTypeRef,
    //            "SetArgumentCount"_W.data(),
    //            setArgumentCountSignature.data(),
    //            setArgumentCountSignature.size(),
    //            &setArgumentCountRef);
    //
    //        helper.LoadLocal(composedIndex);
    //        helper.LoadInt32(target.Signature.NumberOfArguments());
    //        helper.CallMember(setArgumentCountRef, false);
    //    }
    //
    //    // AddParameter
    //    {
    //        std::vector<BYTE> addParameterSignature = {
    //            IMAGE_CEE_CS_CALLCONV_HASTHIS,
    //            2,
    //            ELEMENT_TYPE_VOID,
    //            ELEMENT_TYPE_I4,
    //            ELEMENT_TYPE_OBJECT };
    //
    //        mdMemberRef addParameterRef;
    //        hr = metadataEmit->DefineMemberRef(
    //            composedInterceptorTypeRef,
    //            "AddParameter"_W.data(),
    //            addParameterSignature.data(),
    //            addParameterSignature.size(),
    //            &addParameterRef);
    //
    //        for (size_t i = 0; i < target.Signature.NumberOfArguments(); i++)
    //        {
    //            helper.LoadLocal(composedIndex);
    //            helper.LoadInt32(i);
    //            helper.LoadArgument(shift + i);
    //
    //            auto argument = target.ResolveParameterType(target.Signature.Arguments[i]);
    //
    //            if (argument.IsRefType)
    //            {
    //                helper.LoadInd(argument.TypeDef);
    //            }
    //
    //            if (argument.IsBoxed)
    //            {
    //                auto token = util::GetTypeToken(metadataEmit, mscorlibRef, argument.Raw);
    //                helper.Box(token);
    //            }
    //
    //            helper.CallMember(addParameterRef, false);
    //        }
    //    }
    //
    //    // AddChild
    //    BYTE compressedToken[10];
    //    ULONG tokenLength = CorSigCompressToken(interceptorInterfaceRef, compressedToken);
    //
    //    std::vector<BYTE> addChildSignature = {
    //        IMAGE_CEE_CS_CALLCONV_HASTHIS,
    //        1,
    //        ELEMENT_TYPE_VOID,
    //        ELEMENT_TYPE_CLASS };
    //
    //    addChildSignature.insert(addChildSignature.end(), compressedToken, compressedToken + tokenLength);
    //
    //    mdMemberRef addChildRef;
    //    hr = metadataEmit->DefineMemberRef(
    //        composedInterceptorTypeRef,
    //        "AddChild"_W.data(),
    //        addChildSignature.data(),
    //        addChildSignature.size(),
    //        &addChildRef);
    //
    //    for (size_t i = 0; i < inteceptionRefs.size(); i++)
    //    {
    //        helper.LoadLocal(composedIndex);
    //        helper.LoadLocal(inteceptionRefs[i].LocalVarIndex);
    //        helper.CallMember(addChildRef, false);
    //    }
    //
    //    // set MethodFinder
    //    if (std::get<1>(methodFinder))
    //    {
    //        tokenLength = CorSigCompressToken(methodFinderInterfaceRef, compressedToken);
    //
    //        std::vector<BYTE> setMethodFinderSignature = {
    //            IMAGE_CEE_CS_CALLCONV_HASTHIS,
    //            1,
    //            ELEMENT_TYPE_VOID,
    //            ELEMENT_TYPE_CLASS };
    //
    //        setMethodFinderSignature.insert(setMethodFinderSignature.end(), compressedToken, compressedToken + tokenLength);
    //
    //        mdMemberRef setMethodFinderRef;
    //        hr = metadataEmit->DefineMemberRef(
    //            composedInterceptorTypeRef,
    //            "set_MethodFinder"_W.data(),
    //            setMethodFinderSignature.data(),
    //            setMethodFinderSignature.size(),
    //            &setMethodFinderRef);
    //
    //        helper.LoadLocal(composedIndex);
    //        helper.LoadLocal(methodFinderIndex);
    //        helper.CallMember(setMethodFinderRef, false);
    //    }
    //
    //    //execute
    //    std::vector<BYTE> executeSignature = {
    //        IMAGE_CEE_CS_CALLCONV_HASTHIS,
    //        0,
    //        ELEMENT_TYPE_OBJECT,
    //    };
    //
    //    mdMemberRef executeRef;
    //    hr = metadataEmit->DefineMemberRef(
    //        composedInterceptorTypeRef,
    //        "Execute"_W.data(),
    //        executeSignature.data(),
    //        executeSignature.size(),
    //        &executeRef);
    //
    //    helper.LoadLocal(composedIndex);
    //    helper.CallMember(executeRef, false);
    //
    //    helper.StLocal(resultIndex);
    //
    //    // GetParameter
    //    std::vector<BYTE> getParameterSignature = {
    //        IMAGE_CEE_CS_CALLCONV_HASTHIS,
    //        1,
    //        ELEMENT_TYPE_OBJECT,
    //        ELEMENT_TYPE_I4 };
    //
    //    mdMemberRef getParameterRef;
    //    hr = metadataEmit->DefineMemberRef(
    //        composedInterceptorTypeRef,
    //        "GetParameter"_W.data(),
    //        getParameterSignature.data(),
    //        getParameterSignature.size(),
    //        &getParameterRef);
    //
    //    for (size_t i = 0; i < target.Signature.NumberOfArguments(); i++)
    //    {
    //        auto argument = target.ResolveParameterType(target.Signature.Arguments[i]);
    //
    //        if (!argument.IsRefType)
    //        {
    //            continue;
    //        }
    //
    //        helper.LoadArgument(shift + i);
    //
    //        helper.LoadLocal(composedIndex);
    //        helper.LoadInt32(i);
    //        helper.CallMember(getParameterRef, false);
    //
    //        auto token = util::GetTypeToken(metadataEmit, mscorlibRef, argument.Raw);
    //
    //        if (argument.IsBoxed)
    //        {
    //            helper.UnboxAny(token);
    //        }
    //
    //        helper.StInd(argument.TypeDef);
    //    }
    //
    //    if (!retType.IsVoid)
    //    {
    //        helper.LoadLocal(resultIndex);
    //    }
    //
    //    // ret
    //    helper.Ret();
    //
    //    hr = rewriter.Export();
    //
    //    return S_OK;
    //}
