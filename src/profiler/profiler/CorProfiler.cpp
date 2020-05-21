#include <algorithm>
#include <iostream>
#include <string>
#include "corhlpr.h"
#include "corhdr.h"
#include "configuration/Configuration.h"
#include "const/const.h"
#include "info/InterceptionVarInfo.h"
#include "info/parser.h"
#include "rewriter/ILRewriter.h"
#include "rewriter/ILRewriterHelper.h"
#include "util/helpers.h"
#include "util/util.h"
#include "util/ComPtr.h"
#include "CorProfiler.h"
#include "dllmain.h"

CorProfiler::CorProfiler() : refCount(0), corProfilerInfo(nullptr)
{
}

CorProfiler::~CorProfiler()
{
    if (this->corProfilerInfo != nullptr)
    {
        this->corProfilerInfo->Release();
        this->corProfilerInfo = nullptr;
    }
}

HRESULT STDMETHODCALLTYPE CorProfiler::Initialize(IUnknown* pICorProfilerInfoUnk)
{
    HRESULT queryInterfaceResult = pICorProfilerInfoUnk->QueryInterface(__uuidof(ICorProfilerInfo8), reinterpret_cast<void**>(&this->corProfilerInfo));

    if (FAILED(queryInterfaceResult))
    {
        return E_FAIL;
    }

    DWORD eventMask = COR_PRF_MONITOR_JIT_COMPILATION |
        COR_PRF_DISABLE_TRANSPARENCY_CHECKS_UNDER_FULL_TRUST | 
        COR_PRF_DISABLE_INLINING | COR_PRF_MONITOR_MODULE_LOADS |
        COR_PRF_DISABLE_ALL_NGEN_IMAGES;

    auto hr = this->corProfilerInfo->SetEventMask(eventMask);

    printEveryCall = GetEnvironmentValue("PROFILER_PRINT_EVERY_CALL"_W) == "true"_W;

    configuration = configuration::Configuration::LoadConfiguration(GetEnvironmentValue("PROFILER_CONFIGURATION"_W));

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::Shutdown()
{
    if (this->corProfilerInfo != nullptr)
    {
        this->corProfilerInfo->Release();
        this->corProfilerInfo = nullptr;
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::AppDomainCreationStarted(AppDomainID appDomainId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::AppDomainCreationFinished(AppDomainID appDomainId, HRESULT hrStatus)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::AppDomainShutdownStarted(AppDomainID appDomainId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::AppDomainShutdownFinished(AppDomainID appDomainId, HRESULT hrStatus)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::AssemblyLoadStarted(AssemblyID assemblyId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::AssemblyLoadFinished(AssemblyID assemblyId, HRESULT hrStatus)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::AssemblyUnloadStarted(AssemblyID assemblyId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::AssemblyUnloadFinished(AssemblyID assemblyId, HRESULT hrStatus)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ModuleLoadStarted(ModuleID moduleId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ModuleLoadFinished(ModuleID moduleId, HRESULT hrStatus)
{
    HRESULT hr;
    const auto module_info = info::ModuleInfo::GetModuleInfo(this->corProfilerInfo, moduleId);
    auto app_domain_id = module_info.assembly.appDomainId;

    ComPtr<IUnknown> metadataInterfaces;
    IfFailRet(this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf()));

    auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);

    mdModule module;
    hr = metadataImport->GetModuleFromScope(&module);

    GUID module_version_id;
    hr = metadataImport->GetScopeProps(nullptr, 0, nullptr, &module_version_id);

    modules[moduleId] = module_version_id;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ModuleUnloadStarted(ModuleID moduleId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ModuleUnloadFinished(ModuleID moduleId, HRESULT hrStatus)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ModuleAttachedToAssembly(ModuleID moduleId, AssemblyID AssemblyId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ClassLoadStarted(ClassID classId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ClassLoadFinished(ClassID classId, HRESULT hrStatus)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ClassUnloadStarted(ClassID classId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ClassUnloadFinished(ClassID classId, HRESULT hrStatus)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::FunctionUnloadStarted(FunctionID functionId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::JITCompilationStarted(FunctionID functionId, BOOL fIsSafeToBlock)
{
    HRESULT hr;
    mdToken functionToken;
    ClassID classId;
    ModuleID moduleId;

    IfFailRet(this->corProfilerInfo->GetFunctionInfo(functionId, &classId, &moduleId, &functionToken));

    auto moduleInfo = info::ModuleInfo::GetModuleInfo(this->corProfilerInfo, moduleId);

    // if the current call is not a call to one of skipped assemblies
    if (SkipAssembly(moduleInfo.assembly.name))
    {
        return S_OK;
    }

    // load once into appdomain
    if (loadedIntoAppDomains.find(moduleInfo.assembly.appDomainId) == loadedIntoAppDomains.end())
    {
        loadedIntoAppDomains.insert(moduleInfo.assembly.appDomainId);

        ComPtr<IUnknown> metadataInterfaces;
        IfFailRet(this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf()));

        const auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);

        auto functionInfo = info::FunctionInfo::GetFunctionInfo(metadataImport, functionToken);

        IfFailRet(hr);

        std::cout << "Load into app_domain_id " << moduleInfo.assembly.appDomainId
            << "Before call to " << ToString(functionInfo.Type.Name) << "." << ToString(functionInfo.Name)
            << " num args " << functionInfo.Signature.NumberOfArguments()
            << " from assembly " << ToString(moduleInfo.assembly.name)
            << std::endl << std::flush;

        return InjectLoadMethod(
            moduleId,
            functionToken);
    }

    return Rewrite(moduleId, functionToken);
}

HRESULT CorProfiler::InjectLoadMethod(ModuleID moduleId, mdMethodDef methodDef)
{
    mdMethodDef retMethodToken;

    auto hr = GenerateLoadMethod(moduleId, retMethodToken);

    rewriter::ILRewriter rewriter(this->corProfilerInfo, nullptr, moduleId, methodDef);
    IfFailRet(rewriter.Import());

    rewriter::ILRewriterHelper helper(&rewriter);
    helper.SetILPosition(rewriter.GetILList()->m_pNext);
    helper.CallMember(retMethodToken, false);
    hr = rewriter.Export();

    return S_OK;
}

HRESULT CorProfiler::GenerateLoadMethod(ModuleID moduleId, mdMethodDef& retMethodToken) {

    HRESULT hr;

    ComPtr<IUnknown> metadataInterfaces;
    IfFailRet(this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf()));

    auto metadataEmit = metadataInterfaces.As<IMetaDataEmit2>(IID_IMetaDataEmit);
    auto metadataAssemblyEmit = metadataInterfaces.As<IMetaDataAssemblyEmit>(IID_IMetaDataAssemblyEmit);

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
      ELEMENT_TYPE_VOID
    };

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
        ELEMENT_TYPE_CLASS
    };
    assemblyLoadFromSignature.insert(assemblyLoadFromSignature.end(), compressedToken, compressedToken + tokenLength);
    assemblyLoadFromSignature.push_back(ELEMENT_TYPE_STRING);

    mdMemberRef assemblyLoadFromRef;
    hr = metadataEmit->DefineMemberRef(
        assemblyTypeRef, _const::LoadFrom.data(),
        assemblyLoadFromSignature.data(),
        assemblyLoadFromSignature.size(),
        &assemblyLoadFromRef);


    rewriter::ILRewriter rewriter(this->corProfilerInfo, nullptr, moduleId, retMethodToken);
    rewriter.InitializeTiny();
    rewriter::ILRewriterHelper helper(&rewriter);
    helper.SetILPosition(rewriter.GetILList()->m_pNext);

    for(const auto& el: configuration.Assemblies)
    {
        // assembly path
        mdString pathToken;
        hr = metadataEmit->DefineUserString(el.c_str(), (ULONG)el.length(), &pathToken);

        helper.LoadStr(pathToken);
        helper.CallMember(assemblyLoadFromRef, false);
        helper.Pop();
    }

    // ret
    helper.Ret();

    hr = rewriter.Export();

    return S_OK;
}

bool CorProfiler::SkipAssembly(const wstring& name)
{
    std::vector<wstring> skipAssemblies{
      "mscorlib"_W,
      "netstandard"_W,
      "System.Core"_W,
      "System.Runtime"_W,
      "System.IO.FileSystem"_W,
      "System.Collections"_W,
      "System.Runtime.Extensions"_W,
      "System.Threading.Tasks"_W,
      "System.Runtime.InteropServices"_W,
      "System.Runtime.InteropServices.RuntimeInformation"_W,
      "System.ComponentModel"_W,
      "System.Console"_W,
      "System.Diagnostics.DiagnosticSource"_W,
      "System.Private.CoreLib"_W,
      "Microsoft.Extensions.Options"_W,
      "Microsoft.Extensions.ObjectPool"_W,
      "System.Configuration"_W,
      "System.Xml.Linq"_W,
      "Microsoft.AspNetCore.Razor.Language"_W,
      "Microsoft.AspNetCore.Mvc.RazorPages"_W,
      "Microsoft.CSharp"_W,
      "Anonymously Hosted DynamicMethods Assembly"_W,
      "ISymWrapper"_W,
      "Interception"_W,
      "Interception.Common"_W,
      "Interception.Observers"_W,
      "StatsdClient"_W,
      "Newtonsoft.Json"_W
    };

    return std::find(skipAssemblies.begin(), skipAssemblies.end(), name) != skipAssemblies.end();
}

HRESULT CorProfiler::Rewrite(ModuleID moduleId, mdToken callerToken)
{
    HRESULT hr;

    rewriter::ILRewriter rewriter(this->corProfilerInfo, nullptr, moduleId, callerToken);
    IfFailRet(rewriter.Import());

    ComPtr<IUnknown> metadataInterfaces;
    IfFailRet(this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf()));

    auto metadataEmit = metadataInterfaces.As<IMetaDataEmit2>(IID_IMetaDataEmit);
    auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);
    auto metadataAssemblyEmit = metadataInterfaces.As<IMetaDataAssemblyEmit>(IID_IMetaDataAssemblyEmit);

    auto moduleInfo = info::ModuleInfo::GetModuleInfo(this->corProfilerInfo, moduleId);

    for (rewriter::ILInstr* pInstr = rewriter.GetILList()->m_pNext;
        pInstr != rewriter.GetILList(); pInstr = pInstr->m_pNext) {
        if (pInstr->m_opcode != rewriter::CEE_CALL && pInstr->m_opcode != rewriter::CEE_CALLVIRT) {
            continue;
        }

        auto target = info::FunctionInfo::GetFunctionInfo(metadataImport, pInstr->m_Arg32);

        auto targetMdToken = pInstr->m_Arg32;

        if (printEveryCall)
        {
            std::cout << "Found call to " << ToString(target.Type.Name) << "." << ToString(target.Name)
            << " num args " << target.Signature.NumberOfArguments()
            << " from assembly " << ToString(moduleInfo.assembly.name)
            << std::endl << std::flush;
        }

        auto interceptions = FindInterceptions(moduleInfo.assembly.name, target);

        if (!interceptions.empty())
        {
            std::cout << "Found call to " << ToString(target.Type.Name) << "." << ToString(target.Name)
                << " num args " << target.Signature.NumberOfArguments()
                << " from assembly " << ToString(moduleInfo.assembly.name)
                << std::endl << std::flush;

            rewriter::ILRewriterHelper helper(&rewriter);
            helper.SetILPosition(pInstr);

            mdMethodDef interceptorRef;
            GenerateInterceptMethod(moduleId, target, interceptions, targetMdToken, interceptorRef);

            helper.CallMember(interceptorRef, false);

            pInstr->m_opcode = rewriter::CEE_NOP;

            IfFailRet(rewriter.Export());
        }
    }

    return S_OK;
}

HRESULT CorProfiler::GenerateInterceptMethod(ModuleID moduleId, info::FunctionInfo& target, const std::vector<configuration::Interceptor>& interceptions, INT32 targetMdToken, mdMethodDef& retMethodToken)
{
    HRESULT hr;

    ComPtr<IUnknown> metadataInterfaces;
    IfFailRet(this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf()));

    auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);
    auto metadataEmit = metadataInterfaces.As<IMetaDataEmit2>(IID_IMetaDataEmit);
    auto assemblyImport = metadataInterfaces.As<IMetaDataAssemblyImport>(IID_IMetaDataEmit);
    auto metadataAssemblyEmit = metadataInterfaces.As<IMetaDataAssemblyEmit>(IID_IMetaDataAssemblyEmit);

    // define mscorlib.dll
    mdModuleRef mscorlibRef;
    GetMsCorLibRef(hr, metadataAssemblyEmit, mscorlibRef);

    // define interception.core.dll
    mdModuleRef baseDllRef;
    GetWrapperRef(hr, metadataAssemblyEmit, baseDllRef, configuration.Base.AssemblyName);

    // Define System.Object
    mdTypeRef objectTypeRef;
    metadataEmit->DefineTypeRefByName(mscorlibRef, _const::SystemObject.data(), &objectTypeRef);

    // Define System.Exception
    mdTypeRef exceptionTypeRef;
    metadataEmit->DefineTypeRefByName(mscorlibRef, _const::SystemException.data(), &exceptionTypeRef);

    // Define a static type
    mdTypeDef newTypeDef;
    auto className = target.Type.Name + target.Name + "Interception"_W;
    std::replace(className.begin(), className.end(), '.'_W, '_'_W);
    hr = metadataEmit->DefineTypeDef(className.c_str(), tdAbstract | tdSealed,
        objectTypeRef, NULL, &newTypeDef);

    // create method sugnature
    std::vector<BYTE> signature = {
        // call convention for static method
        IMAGE_CEE_CS_CALLCONV_DEFAULT,
        // number of arguments: original count + this if instance method
        (BYTE)(target.Signature.NumberOfArguments() + (target.Signature.IsInstanceMethod() ? 1 : 0))
    };
    // return type
    auto retType = target.ResolveParameterType(target.Signature.ReturnType);
    signature.insert(signature.end(), retType.Raw.begin(), retType.Raw.end());
    
    // insert this
    if (target.Signature.IsInstanceMethod())
    {
        signature.push_back(ELEMENT_TYPE_OBJECT);
    }

    target.Signature.ParseArguments();

    // insert existing arguments
    for (size_t i = 0; i < target.Signature.NumberOfArguments(); i++)
    {
        auto argument = target.ResolveParameterType(target.Signature.Arguments[i]);
        if (argument.IsRefType)
        {
            signature.push_back(ELEMENT_TYPE_BYREF);
        }

        signature.push_back(argument.TypeDef);
    }
    
    // define a method
    hr = metadataEmit->DefineMethod(newTypeDef,
        "__InterceptionMethod__"_W.c_str(),
        mdStatic | mdPublic | miAggressiveInlining,
        signature.data(),
        signature.size(),
        0,
        0,
        &retMethodToken);

    rewriter::ILRewriter rewriter(this->corProfilerInfo, nullptr, moduleId, retMethodToken);
    rewriter::ILRewriterHelper helper(&rewriter);

    mdSignature localSigToken;
    std::vector<BYTE> localSig = {
        IMAGE_CEE_CS_CALLCONV_LOCAL_SIG,
        1,
        ELEMENT_TYPE_OBJECT,
    };

    int resultIndex = 0;

    hr = metadataEmit->GetTokenFromSig(localSig.data(), localSig.size(), &localSigToken);

    rewriter.InitializeTiny();
    rewriter.SetTkLocalVarSig(localSigToken);
    helper.SetILPosition(rewriter.GetILList()->m_pNext);

    // define baseType = IInterceptor
    mdTypeRef baseTypeRef;
    hr = metadataEmit->DefineTypeRefByName(
        baseDllRef,
        configuration.Base.TypeName.data(),
        &baseTypeRef);
    IfFailRet(hr);

    // define composedType = ComposedInterceptor
    mdTypeRef composedTypeRef;
    hr = metadataEmit->DefineTypeRefByName(
        baseDllRef,
        configuration.Composed.TypeName.data(),
        &composedTypeRef);
    IfFailRet(hr);

    // local sig
    std::vector<info::InterceptionVarInfo> inteceptionRefs{};
    for (const auto& interception : interceptions)
    {
        // define interception.dll
        mdModuleRef interceptorDllRef;
        GetWrapperRef(hr, metadataAssemblyEmit, interceptorDllRef, interception.AssemblyName);

        // define wrappedType
        mdTypeRef interceptorTypeRef;
        hr = metadataEmit->DefineTypeRefByName(
            interceptorDllRef,
            interception.TypeName.data(),
            &interceptorTypeRef);
        IfFailRet(hr);

        int locaVarIndex = 0;
        helper.AddLocalVariable(interceptorTypeRef, locaVarIndex);
        inteceptionRefs.push_back(info::InterceptionVarInfo(interceptorTypeRef, locaVarIndex));
    }

    int composedIndex = 0;
    helper.AddLocalVariable(composedTypeRef, composedIndex);

    // ctor
    for (const auto& interceptor : inteceptionRefs)
    {
        std::vector<BYTE> ctorSignature = {
            IMAGE_CEE_CS_CALLCONV_HASTHIS,
            0,
            ELEMENT_TYPE_VOID
        };

        mdMemberRef ctorRef;
        hr = metadataEmit->DefineMemberRef(
            interceptor.TypeRef,
            _const::ctor.data(),
            ctorSignature.data(),
            ctorSignature.size(),
            &ctorRef);

        helper.NewObject(ctorRef);
        helper.StLocal(interceptor.LocalVarIndex);
    }

    {
        std::vector<BYTE> ctorSignature = {
            IMAGE_CEE_CS_CALLCONV_HASTHIS,
            0,
            ELEMENT_TYPE_VOID
        };

        mdMemberRef ctorRef;
        hr = metadataEmit->DefineMemberRef(
            composedTypeRef,
            _const::ctor.data(),
            ctorSignature.data(),
            ctorSignature.size(),
            &ctorRef);

        helper.NewObject(ctorRef);
        helper.StLocal(composedIndex);
    }

    auto shift = 0;

    //SetThis
    if (target.Signature.IsInstanceMethod())
    {
        std::vector<BYTE> setThisSignature = {
            IMAGE_CEE_CS_CALLCONV_HASTHIS,
            1,
            ELEMENT_TYPE_VOID,
            ELEMENT_TYPE_OBJECT
        };

        mdMemberRef setThisRef;
        hr = metadataEmit->DefineMemberRef(
            composedTypeRef,
            "set_This"_W.data(),
            setThisSignature.data(),
            setThisSignature.size(),
            &setThisRef);

        shift = 1;
        helper.LoadLocal(composedIndex);
        helper.LoadArgument(0);
        helper.CallMember(setThisRef, false);
    }

    //SetMdToken
    {
        std::vector<BYTE> setMdTokenSignature = {
            IMAGE_CEE_CS_CALLCONV_HASTHIS,
            1,
            ELEMENT_TYPE_VOID,
            ELEMENT_TYPE_I4
        };

        mdMemberRef setMdTokenRef;
        hr = metadataEmit->DefineMemberRef(
            composedTypeRef,
            "set_MdToken"_W.data(),
            setMdTokenSignature.data(),
            setMdTokenSignature.size(),
            &setMdTokenRef);

        helper.LoadLocal(composedIndex);
        helper.LoadInt32(targetMdToken);
        helper.CallMember(setMdTokenRef, false);
    }

    //SetModuleVersionPtr
    {
        std::vector<BYTE> setModuleVersionPtrSignature = {
            IMAGE_CEE_CS_CALLCONV_HASTHIS,
            1,
            ELEMENT_TYPE_VOID,
            ELEMENT_TYPE_I8
        };

        mdMemberRef setModuleVersionPtrRef;
        hr = metadataEmit->DefineMemberRef(
            composedTypeRef,
            "set_ModuleVersionPtr"_W.data(),
            setModuleVersionPtrSignature.data(),
            setModuleVersionPtrSignature.size(),
            &setModuleVersionPtrRef);

        helper.LoadLocal(composedIndex);
        const void* module_version_id_ptr = &modules[moduleId];
        helper.LoadInt64(reinterpret_cast<INT64>(module_version_id_ptr));
        helper.CallMember(setModuleVersionPtrRef, false);
    }

    //SetArgumentCount
    {
        std::vector<BYTE> setArgumentCountSignature = {
            IMAGE_CEE_CS_CALLCONV_HASTHIS,
            1,
            ELEMENT_TYPE_VOID,
            ELEMENT_TYPE_I4
        };

        mdMemberRef setArgumentCountRef;
        hr = metadataEmit->DefineMemberRef(
            composedTypeRef,
            "SetArgumentCount"_W.data(),
            setArgumentCountSignature.data(),
            setArgumentCountSignature.size(),
            &setArgumentCountRef);

        helper.LoadLocal(composedIndex);
        helper.LoadInt32(target.Signature.NumberOfArguments());
        helper.CallMember(setArgumentCountRef, false);
    }

    // AddParameter
    {
        std::vector<BYTE> addParameterSignature = {
            IMAGE_CEE_CS_CALLCONV_HASTHIS,
            2,
            ELEMENT_TYPE_VOID,
            ELEMENT_TYPE_I4,
            ELEMENT_TYPE_OBJECT
        };

        mdMemberRef addParameterRef;
        hr = metadataEmit->DefineMemberRef(
            composedTypeRef,
            "AddParameter"_W.data(),
            addParameterSignature.data(),
            addParameterSignature.size(),
            &addParameterRef);

        for (size_t i = 0; i < target.Signature.NumberOfArguments(); i++)
        {
            helper.LoadLocal(composedIndex);
            helper.LoadInt32(i);
            helper.LoadArgument(shift + i);

            auto argument = target.ResolveParameterType(target.Signature.Arguments[i]);

            if (argument.IsRefType)
            {
                helper.LoadInd(argument.TypeDef);
            }

            if (argument.IsBoxed)
            {
                auto token = util::GetTypeToken(metadataEmit, mscorlibRef, argument.Raw);
                helper.Box(token);
            }

            helper.CallMember(addParameterRef, false);
        }
    }

    // AddChild
    BYTE compressedToken[10];
    ULONG tokenLength = CorSigCompressToken(baseTypeRef, compressedToken);

    std::vector<BYTE> addChildSignature = {
            IMAGE_CEE_CS_CALLCONV_HASTHIS,
            1,
            ELEMENT_TYPE_VOID,
            ELEMENT_TYPE_CLASS
    };

    addChildSignature.insert(addChildSignature.end(), compressedToken, compressedToken + tokenLength);

    mdMemberRef addChildRef;
    hr = metadataEmit->DefineMemberRef(
        composedTypeRef,
        "AddChild"_W.data(),
        addChildSignature.data(),
        addChildSignature.size(),
        &addChildRef);

    for (size_t i = 0; i < inteceptionRefs.size(); i++)
    {
        helper.LoadLocal(composedIndex);
        helper.LoadLocal(inteceptionRefs[i].LocalVarIndex);
        helper.Cast(baseTypeRef);
        helper.CallMember(addChildRef, false);
    }

    //execute
    std::vector<BYTE> executeSignature = {
            IMAGE_CEE_CS_CALLCONV_HASTHIS,
            0,
            ELEMENT_TYPE_OBJECT,
    };

    mdMemberRef executeRef;
    hr = metadataEmit->DefineMemberRef(
        composedTypeRef,
        "Execute"_W.data(),
        executeSignature.data(),
        executeSignature.size(),
        &executeRef);

    helper.LoadLocal(composedIndex);
    helper.CallMember(executeRef, false);

    helper.StLocal(resultIndex);

    // GetParameter
    std::vector<BYTE> getParameterSignature = {
        IMAGE_CEE_CS_CALLCONV_HASTHIS,
        1,
        ELEMENT_TYPE_OBJECT,
        ELEMENT_TYPE_I4
    };

    mdMemberRef getParameterRef;
    hr = metadataEmit->DefineMemberRef(
        composedTypeRef,
        "GetParameter"_W.data(),
        getParameterSignature.data(),
        getParameterSignature.size(),
        &getParameterRef);

    for (size_t i = 0; i < target.Signature.NumberOfArguments(); i++)
    {
        auto argument = target.ResolveParameterType(target.Signature.Arguments[i]);

        if (!argument.IsRefType)
        {
            continue;
        }

        helper.LoadArgument(shift + i);

        helper.LoadLocal(composedIndex);
        helper.LoadInt32(i);
        helper.CallMember(getParameterRef, false);

        auto token = util::GetTypeToken(metadataEmit, mscorlibRef, argument.Raw);

        if (argument.IsBoxed)
        {
            helper.UnboxAny(token);
        }

        helper.StInd(argument.TypeDef);
    }

    if (!retType.IsVoid)
    {
        helper.LoadLocal(resultIndex);
    }

    // ret
    helper.Ret();

    hr = rewriter.Export();

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::JITCompilationFinished(FunctionID functionId, HRESULT hrStatus, BOOL fIsSafeToBlock)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::JITCachedFunctionSearchStarted(FunctionID functionId, BOOL* pbUseCachedFunction)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::JITCachedFunctionSearchFinished(FunctionID functionId, COR_PRF_JIT_CACHE result)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::JITFunctionPitched(FunctionID functionId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::JITInlining(FunctionID callerId, FunctionID calleeId, BOOL* pfShouldInline)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ThreadCreated(ThreadID threadId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ThreadDestroyed(ThreadID threadId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ThreadAssignedToOSThread(ThreadID managedThreadId, DWORD osThreadId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RemotingClientInvocationStarted()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RemotingClientSendingMessage(GUID* pCookie, BOOL fIsAsync)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RemotingClientReceivingReply(GUID* pCookie, BOOL fIsAsync)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RemotingClientInvocationFinished()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RemotingServerReceivingMessage(GUID* pCookie, BOOL fIsAsync)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RemotingServerInvocationStarted()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RemotingServerInvocationReturned()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RemotingServerSendingReply(GUID* pCookie, BOOL fIsAsync)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::UnmanagedToManagedTransition(FunctionID functionId, COR_PRF_TRANSITION_REASON reason)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ManagedToUnmanagedTransition(FunctionID functionId, COR_PRF_TRANSITION_REASON reason)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RuntimeSuspendStarted(COR_PRF_SUSPEND_REASON suspendReason)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RuntimeSuspendFinished()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RuntimeSuspendAborted()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RuntimeResumeStarted()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RuntimeResumeFinished()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RuntimeThreadSuspended(ThreadID threadId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RuntimeThreadResumed(ThreadID threadId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::MovedReferences(ULONG cMovedObjectIDRanges, ObjectID oldObjectIDRangeStart[], ObjectID newObjectIDRangeStart[], ULONG cObjectIDRangeLength[])
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ObjectAllocated(ObjectID objectId, ClassID classId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ObjectsAllocatedByClass(ULONG cClassCount, ClassID classIds[], ULONG cObjects[])
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ObjectReferences(ObjectID objectId, ClassID classId, ULONG cObjectRefs, ObjectID objectRefIds[])
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RootReferences(ULONG cRootRefs, ObjectID rootRefIds[])
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionThrown(ObjectID thrownObjectId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionSearchFunctionEnter(FunctionID functionId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionSearchFunctionLeave()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionSearchFilterEnter(FunctionID functionId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionSearchFilterLeave()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionSearchCatcherFound(FunctionID functionId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionOSHandlerEnter(UINT_PTR __unused)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionOSHandlerLeave(UINT_PTR __unused)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionUnwindFunctionEnter(FunctionID functionId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionUnwindFunctionLeave()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionUnwindFinallyEnter(FunctionID functionId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionUnwindFinallyLeave()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionCatcherEnter(FunctionID functionId, ObjectID objectId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionCatcherLeave()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::COMClassicVTableCreated(ClassID wrappedClassId, REFGUID implementedIID, void* pVTable, ULONG cSlots)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::COMClassicVTableDestroyed(ClassID wrappedClassId, REFGUID implementedIID, void* pVTable)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionCLRCatcherFound()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ExceptionCLRCatcherExecute()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ThreadNameChanged(ThreadID threadId, ULONG cchName, WCHAR name[])
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::GarbageCollectionStarted(int cGenerations, BOOL generationCollected[], COR_PRF_GC_REASON reason)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::SurvivingReferences(ULONG cSurvivingObjectIDRanges, ObjectID objectIDRangeStart[], ULONG cObjectIDRangeLength[])
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::GarbageCollectionFinished()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::FinalizeableObjectQueued(DWORD finalizerFlags, ObjectID objectID)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::RootReferences2(ULONG cRootRefs, ObjectID rootRefIds[], COR_PRF_GC_ROOT_KIND rootKinds[], COR_PRF_GC_ROOT_FLAGS rootFlags[], UINT_PTR rootIds[])
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::HandleCreated(GCHandleID handleId, ObjectID initialObjectId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::HandleDestroyed(GCHandleID handleId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::InitializeForAttach(IUnknown* pCorProfilerInfoUnk, void* pvClientData, UINT cbClientData)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ProfilerAttachComplete()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ProfilerDetachSucceeded()
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ReJITCompilationStarted(FunctionID functionId, ReJITID rejitId, BOOL fIsSafeToBlock)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::GetReJITParameters(ModuleID moduleId, mdMethodDef methodId, ICorProfilerFunctionControl* pFunctionControl)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ReJITCompilationFinished(FunctionID functionId, ReJITID rejitId, HRESULT hrStatus, BOOL fIsSafeToBlock)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ReJITError(ModuleID moduleId, mdMethodDef methodId, FunctionID functionId, HRESULT hrStatus)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::MovedReferences2(ULONG cMovedObjectIDRanges, ObjectID oldObjectIDRangeStart[], ObjectID newObjectIDRangeStart[], SIZE_T cObjectIDRangeLength[])
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::SurvivingReferences2(ULONG cSurvivingObjectIDRanges, ObjectID objectIDRangeStart[], SIZE_T cObjectIDRangeLength[])
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ConditionalWeakTableElementReferences(ULONG cRootRefs, ObjectID keyRefIds[], ObjectID valueRefIds[], GCHandleID rootIds[])
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::GetAssemblyReferences(const WCHAR* wszAssemblyPath, ICorProfilerAssemblyReferenceProvider* pAsmRefProvider)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ModuleInMemorySymbolsUpdated(ModuleID moduleId)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::DynamicMethodJITCompilationStarted(FunctionID functionId, BOOL fIsSafeToBlock, LPCBYTE ilHeader, ULONG cbILHeader)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::DynamicMethodJITCompilationFinished(FunctionID functionId, HRESULT hrStatus, BOOL fIsSafeToBlock)
{
    return S_OK;
}

std::vector<configuration::Interceptor> CorProfiler::FindInterceptions(const wstring& callerAssemblyName, const info::FunctionInfo& target)
{
    std::vector<configuration::StrictInterception> strict{};
    std::copy_if(
        configuration.StrictInterceptions.begin(),
        configuration.StrictInterceptions.end(),
        std::back_inserter(strict),
        [&callerAssemblyName,&target](const configuration::StrictInterception& interception) {
            return (callerAssemblyName == interception.CallerAssemblyName || interception.CallerAssemblyName.empty())
                && target.Type.Name == interception.Target.TypeName
                && target.Name == interception.Target.MethodName && target.Signature.NumberOfArguments() == interception.Target.MethodParametersCount;
        }
    );

    std::vector<configuration::AttributedInterceptor> attributed{};
    std::copy_if(
        configuration.AttributedInterceptors.begin(),
        configuration.AttributedInterceptors.end(),
        std::back_inserter(attributed),
        [&target](const configuration::AttributedInterceptor& interception) {
            return std::find_if(target.Attributes.begin(), target.Attributes.end(), [&interception](const wstring& info) { return info == interception.AttributeType; }) != target.Attributes.end();
        }
    );

    std::vector<configuration::Interceptor> result{};

    for (const auto& el : strict)
    {
        result.push_back(el.Interceptor);
    }

    for (const auto& el : attributed)
    {
        result.push_back(el.Interceptor);
    }

    return result;
}
