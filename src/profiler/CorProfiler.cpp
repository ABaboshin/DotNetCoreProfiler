#include <algorithm>
#include <iostream>
#include <string>
#include "CorProfiler.h"
#include "corhlpr.h"
#include "corhdr.h"
#include "ILRewriter.h"
#include "helpers.h"
#include "util.h"
#include "ComPtr.h"
#include "ILRewriterHelper.h"
#include "clr_const.h"
#include "Configuration.h"
#include "dllmain.h"

CorProfiler* profiler = nullptr;

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
    loaderClass = GetInterceptionLoaderClassName();

    profiler = this;

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
    const auto module_info = GetModuleInfo(this->corProfilerInfo, moduleId);
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

    auto moduleInfo = GetModuleInfo(this->corProfilerInfo, moduleId);

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

        auto functionInfo = GetFunctionInfo(metadataImport, functionToken);

        hr = functionInfo.signature.TryParse();
        IfFailRet(hr);

        std::cout << "Load into app_domain_id " << moduleInfo.assembly.appDomainId
            << "Before call to " << ToString(functionInfo.type.name) << "." << ToString(functionInfo.name)
            << " num args " << functionInfo.signature.NumberOfArguments()
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

    auto hr = GenerateLoadMethod(moduleId, &retMethodToken);

    ILRewriter rewriter(this->corProfilerInfo, nullptr, moduleId, methodDef);
    IfFailRet(rewriter.Import());

    ILRewriterHelper helper(&rewriter);
    helper.SetILPosition(rewriter.GetILList()->m_pNext);
    helper.CallMember(retMethodToken, false);
    hr = rewriter.Export(false);

    return S_OK;
}

HRESULT CorProfiler::GenerateLoadMethod(ModuleID moduleId,
    mdMethodDef* retMethodToken) {

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
    IfFailRet(hr);

    // Define System.Object
    mdTypeRef objectTypeRef;
    metadataEmit->DefineTypeRefByName(mscorlibRef, SystemObject.data(), &objectTypeRef);

    // Define System.Type
    mdTypeRef typeRef;
    metadataEmit->DefineTypeRefByName(mscorlibRef, SystemType.data(), &typeRef);

    // Define System.Activator
    mdTypeRef activatorTypeRef;
    metadataEmit->DefineTypeRefByName(mscorlibRef, SystemActivator.data(), &activatorTypeRef);

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
        retMethodToken);

    // call GetAssemblyBytes
    mdMethodDef getAssemblyMethodDef;
    COR_SIGNATURE getAssemblyBytesSignature[] = {
        IMAGE_CEE_CS_CALLCONV_DEFAULT,
        2,
        ELEMENT_TYPE_VOID,
        ELEMENT_TYPE_BYREF,
        ELEMENT_TYPE_I,
        ELEMENT_TYPE_BYREF,
        ELEMENT_TYPE_I4,
    };

    hr = metadataEmit->DefineMethod(
        newTypeDef, "GetAssemblyBytes"_W.c_str(), mdStatic | mdPinvokeImpl | mdHideBySig,
        getAssemblyBytesSignature, sizeof(getAssemblyBytesSignature), 0, 0,
        &getAssemblyMethodDef);

    metadataEmit->SetMethodImplFlags(getAssemblyMethodDef, miPreserveSig);

#ifdef _WIN32
    wstring nativeProfilerLib = "DotNetCoreProfiler.dll"_W;
#else // _WIN32
    wstring nativeProfilerLib = "DotNetCoreProfiler.so"_W;
#endif // _WIN32

    mdModuleRef profilerRef;
    hr = metadataEmit->DefineModuleRef(nativeProfilerLib.c_str(),
        &profilerRef);

    hr = metadataEmit->DefinePinvokeMap(getAssemblyMethodDef,
        0,
        "GetAssemblyBytes"_W.c_str(),
        profilerRef);

    // System.Byte
    mdTypeRef byteTypeRef;
    hr = metadataEmit->DefineTypeRefByName(mscorlibRef,
        SystemByte.data(),
        &byteTypeRef);

    // System.Runtime.InteropServices.Marshal
    mdTypeRef marshalTypeRef;
    hr = metadataEmit->DefineTypeRefByName(mscorlibRef,
        SystemRuntimeInteropServicesMarshal.data(),
        &marshalTypeRef);

    // System.Runtime.InteropServices.Marshal.Copy
    mdMemberRef marshalCopyMemberRef;
    COR_SIGNATURE marshal_copy_signature[] = {
        IMAGE_CEE_CS_CALLCONV_DEFAULT,
        4,
        ELEMENT_TYPE_VOID,
        ELEMENT_TYPE_I,
        ELEMENT_TYPE_SZARRAY,
        ELEMENT_TYPE_U1,
        ELEMENT_TYPE_I4,
        ELEMENT_TYPE_I4
    };
    hr = metadataEmit->DefineMemberRef(
        marshalTypeRef, Copy.data(), marshal_copy_signature,
        sizeof(marshal_copy_signature), &marshalCopyMemberRef);

    // System.Reflection.Assembly
    mdTypeRef assemblyTypeRef;
    hr = metadataEmit->DefineTypeRefByName(mscorlibRef,
        SystemReflectionAssembly.data(),
        &assemblyTypeRef);

    // System.AppDomain
    mdTypeRef appdomainTypeRef;
    hr = metadataEmit->DefineTypeRefByName(mscorlibRef,
        SystemAppDomain.data(),
        &appdomainTypeRef);

    // System.AppDomain.get_CurrentDomain()
    COR_SIGNATURE getCurrentDomainSignatureStart[] = {
        IMAGE_CEE_CS_CALLCONV_DEFAULT,
        0,
        ELEMENT_TYPE_CLASS,
    };
    ULONG startLength = sizeof(getCurrentDomainSignatureStart);
    ULONG endLength = 0;

    BYTE appdomainTypeRefCompressedToken[4];
    ULONG tokenLength = CorSigCompressToken(appdomainTypeRef, appdomainTypeRefCompressedToken);

    COR_SIGNATURE* getCurrentDomainSignature = new COR_SIGNATURE[startLength + tokenLength];
    memcpy(getCurrentDomainSignature,
        getCurrentDomainSignatureStart,
        startLength);
    memcpy(&getCurrentDomainSignature[startLength],
        appdomainTypeRefCompressedToken,
        tokenLength);

    mdMemberRef getCurrentDomainMethodRef;
    hr = metadataEmit->DefineMemberRef(
        appdomainTypeRef,
        get_CurrentDomain.data(),
        getCurrentDomainSignature,
        startLength + tokenLength,
        &getCurrentDomainMethodRef);
    delete[] getCurrentDomainSignature;

    // Assembly.GetType
    COR_SIGNATURE assemblyGetTypeSignatureStart[] = {
        IMAGE_CEE_CS_CALLCONV_HASTHIS,
        1,
        ELEMENT_TYPE_CLASS
    };

    COR_SIGNATURE assemblyGetTypeSignatureEnd[] = {
        ELEMENT_TYPE_STRING
    };

    startLength = sizeof(assemblyGetTypeSignatureStart);
    endLength = sizeof(assemblyGetTypeSignatureEnd);

    BYTE typeRefCompressedToken[4];
    tokenLength = CorSigCompressToken(typeRef, typeRefCompressedToken);

    COR_SIGNATURE* assemblyGetTypeSignature = new COR_SIGNATURE[startLength + tokenLength + endLength];
    memcpy(assemblyGetTypeSignature,
        assemblyGetTypeSignatureStart,
        startLength);
    memcpy(&assemblyGetTypeSignature[startLength],
        typeRefCompressedToken,
        tokenLength);
    memcpy(&assemblyGetTypeSignature[startLength + tokenLength],
        assemblyGetTypeSignatureEnd,
        endLength);

    mdMemberRef assemblyGetTypeMemberRef;
    hr = metadataEmit->DefineMemberRef(
        assemblyTypeRef, GetType.data(),
        assemblyGetTypeSignature,
        startLength + tokenLength + endLength,
        &assemblyGetTypeMemberRef);
    delete[] assemblyGetTypeSignature;

    // AppDomain.Load
    COR_SIGNATURE appdomainLoadSignatureStart[] = {
        IMAGE_CEE_CS_CALLCONV_HASTHIS,
        1,
        ELEMENT_TYPE_CLASS
    };
    COR_SIGNATURE appdomainLoadSignatureEnd[] = {
        ELEMENT_TYPE_SZARRAY,
        ELEMENT_TYPE_U1
    };
    startLength = sizeof(appdomainLoadSignatureStart);
    endLength = sizeof(appdomainLoadSignatureEnd);

    BYTE assemblyTypeRefCompressedToken[4];
    tokenLength = CorSigCompressToken(assemblyTypeRef, assemblyTypeRefCompressedToken);

    COR_SIGNATURE* appdomainLoadSignature = new COR_SIGNATURE[startLength + tokenLength + endLength];
    memcpy(appdomainLoadSignature,
        appdomainLoadSignatureStart,
        startLength);
    memcpy(&appdomainLoadSignature[startLength],
        assemblyTypeRefCompressedToken,
        tokenLength);
    memcpy(&appdomainLoadSignature[startLength + tokenLength],
        appdomainLoadSignatureEnd,
        endLength);

    mdMemberRef appdomainLoadMemberRef;
    hr = metadataEmit->DefineMemberRef(
        appdomainTypeRef, Load.data(),
        appdomainLoadSignature,
        startLength + tokenLength + endLength,
        &appdomainLoadMemberRef);
    delete[] appdomainLoadSignature;

    // Activator.CreateInstance
    COR_SIGNATURE activatorCreateInstanceSignatureStart[] = {
        IMAGE_CEE_CS_CALLCONV_DEFAULT,
        2,
        ELEMENT_TYPE_OBJECT,
        ELEMENT_TYPE_CLASS
    };
    COR_SIGNATURE activatorCreateInstanceSignatureEnd[] = {
        ELEMENT_TYPE_SZARRAY,
        ELEMENT_TYPE_OBJECT
    };
    startLength = sizeof(activatorCreateInstanceSignatureStart);
    endLength = sizeof(activatorCreateInstanceSignatureEnd);

    /*BYTE typeRefCompressedToken[4];*/
    tokenLength = CorSigCompressToken(typeRef, typeRefCompressedToken);

    COR_SIGNATURE* activatorCreateInstanceSignature = new COR_SIGNATURE[startLength + tokenLength + endLength];
    memcpy(activatorCreateInstanceSignature,
        activatorCreateInstanceSignatureStart,
        startLength);
    memcpy(&activatorCreateInstanceSignature[startLength],
        typeRefCompressedToken,
        tokenLength);
    memcpy(&activatorCreateInstanceSignature[startLength + tokenLength],
        activatorCreateInstanceSignatureEnd,
        endLength);

    mdMemberRef activatorCreateInstanceMemberRef;
    hr = metadataEmit->DefineMemberRef(
        activatorTypeRef, CreateInstance.data(),
        activatorCreateInstanceSignature,
        startLength + tokenLength + endLength,
        &activatorCreateInstanceMemberRef);
    delete[] activatorCreateInstanceSignature;

    // Assembly.CreateInstance
    COR_SIGNATURE assemblyCreateInstanceSignature[] = {
        IMAGE_CEE_CS_CALLCONV_HASTHIS,
        1,
        ELEMENT_TYPE_OBJECT,
        ELEMENT_TYPE_STRING
    };

    mdMemberRef assemblyCreateInstanceMemberRef;
    hr = metadataEmit->DefineMemberRef(
        assemblyTypeRef, CreateInstance.data(),
        assemblyCreateInstanceSignature,
        sizeof(assemblyCreateInstanceSignature),
        &assemblyCreateInstanceMemberRef);

    // loader class
    mdString loaderClassToken;
    hr = metadataEmit->DefineUserString(loaderClass.c_str(), (ULONG)loaderClass.length(),
        &loaderClassToken);

    // loader class param
    auto profilerInterceptionDlls = GetEnvironmentValue("PROFILER_INTERCEPTION_DLLS"_W);
    mdString paramToken;
    hr = metadataEmit->DefineUserString(profilerInterceptionDlls.c_str(), (ULONG)profilerInterceptionDlls.length(),
        &paramToken);

    // local sig
    mdSignature localSigToken;
    COR_SIGNATURE localSig[/*11*/] = {
        IMAGE_CEE_CS_CALLCONV_LOCAL_SIG,
        4,
        ELEMENT_TYPE_I, // assemblyPtr
        ELEMENT_TYPE_I4, // assemblySize
        ELEMENT_TYPE_SZARRAY, // assemblyBytes
        ELEMENT_TYPE_U1,
        ELEMENT_TYPE_SZARRAY, // ctor params
        ELEMENT_TYPE_OBJECT
    };

    hr = metadataEmit->GetTokenFromSig(localSig, sizeof(localSig),
        &localSigToken);

    ILRewriter rewriter(this->corProfilerInfo, nullptr, moduleId, *retMethodToken);
    rewriter.InitializeTiny();
    rewriter.SetTkLocalVarSig(localSigToken);
    ILRewriterHelper helper(&rewriter);
    helper.SetILPosition(rewriter.GetILList()->m_pNext);

    // load addr of assemblyPtr
    helper.LoadLocalAddress(0);
    // load addr of assemblySize
    helper.LoadLocalAddress(1);
    // call GetAssemblyBytes
    helper.CallMember(getAssemblyMethodDef, false);

    // load assemblySize
    helper.LoadLocal(1);

    // create newarr of bytes
    helper.CreateArray(byteTypeRef, 0);

    // set assemblyBytes to newarr
    helper.StLocal(2);

    // fillout params
    helper.CreateArray(objectTypeRef, 1);
    helper.BeginLoadValueIntoArray(0);
    helper.LoadStr(paramToken);
    helper.EndLoadValueIntoArray();

    // set params to newarr
    helper.StLocal(3);

    // load assemblyPtr
    helper.LoadLocal(0);
    // load assemblyBytes
    helper.LoadLocal(2);

    // load 0
    helper.LoadInt32(0);

    // load assemblySize
    helper.LoadLocal(1);

    // call Marshal.Copy
    helper.CallMember(marshalCopyMemberRef, false);

    // call System.AppDomain.CurrentDomain
    helper.CallMember(getCurrentDomainMethodRef, false);

    // load assemblyBytes
    helper.LoadLocal(2);

    // call System.AppDomain.Load
    helper.CallMember(appdomainLoadMemberRef, true);

    // load loaderClassToken
    helper.LoadStr(loaderClassToken);

    // call Assembly.GetType
    helper.CallMember(assemblyGetTypeMemberRef, true);

    // load ctor params
    helper.LoadLocal(3);

    // call Activator.CreateInstance
    helper.CallMember(activatorCreateInstanceMemberRef, false);

    // pop the object
    helper.Pop();

    // ret
    helper.Ret();

    hr = rewriter.Export(false);

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
      "Interception.Executor"_W,
      "Interception.Generator"_W,
      "Interception.Metrics"_W,
      "Interception.Observers"_W,
      "StatsdClient"_W,
      "Newtonsoft.Json"_W
    };

    return std::find(skipAssemblies.begin(), skipAssemblies.end(), name) != skipAssemblies.end();
}

HRESULT CorProfiler::Rewrite(ModuleID moduleId, mdToken callerToken)
{
    HRESULT hr;

    ILRewriter rewriter(this->corProfilerInfo, nullptr, moduleId, callerToken);
    IfFailRet(rewriter.Import());

    ComPtr<IUnknown> metadataInterfaces;
    IfFailRet(this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf()));

    auto metadataEmit = metadataInterfaces.As<IMetaDataEmit2>(IID_IMetaDataEmit);
    auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);
    auto metadataAssemblyEmit = metadataInterfaces.As<IMetaDataAssemblyEmit>(IID_IMetaDataAssemblyEmit);

    auto moduleInfo = GetModuleInfo(this->corProfilerInfo, moduleId);

    if (!SkipAssembly(moduleInfo.assembly.name)) for (ILInstr* pInstr = rewriter.GetILList()->m_pNext;
        pInstr != rewriter.GetILList(); pInstr = pInstr->m_pNext) {
        if (pInstr->m_opcode != CEE_CALL && pInstr->m_opcode != CEE_CALLVIRT) {
            continue;
        }

        auto target = GetFunctionInfo(metadataImport, pInstr->m_Arg32);
        target.signature.TryParse();

        auto targetMdToken = pInstr->m_Arg32;

        if (printEveryCall)
        {
            std::cout << "Found call to " << ToString(target.type.name) << "." << ToString(target.name)
            << " num args " << target.signature.NumberOfArguments()
            << " from assembly " << ToString(moduleInfo.assembly.name)
            << std::endl << std::flush;
        }

        for (const auto& interception : configuration.interceptions)
        {
            if (
                (moduleInfo.assembly.name == interception.callerAssemblyName || interception.callerAssemblyName.empty())
                && target.type.name == interception.Target.TypeName
                && target.name == interception.Target.MethodName && interception.Target.MethodParametersCount == target.signature.NumberOfArguments()
                )
            {
                std::cout << "Found call to " << ToString(target.type.name) << "." << ToString(target.name)
                    << " num args " << target.signature.NumberOfArguments()
                    << " from assembly " << ToString(moduleInfo.assembly.name)
                    << std::endl << std::flush;

                ILRewriterHelper helper(&rewriter);
                helper.SetILPosition(pInstr);

                mdMethodDef interceptorRef;
                GenerateInterceptMethod(moduleId, target, interception, targetMdToken, &interceptorRef);

                helper.CallMember(interceptorRef, false);

                pInstr->m_opcode = CEE_NOP;
                            
                IfFailRet(rewriter.Export(false));
            }
        }
    }

    return S_OK;
}

HRESULT CorProfiler::GenerateInterceptMethod(ModuleID moduleId, const FunctionInfo& target, const Interception& interception, INT32 targetMdToken, mdMethodDef* retMethodToken)
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
    IfFailRet(hr);

    // Define System.Object
    mdTypeRef objectTypeRef;
    metadataEmit->DefineTypeRefByName(mscorlibRef, SystemObject.data(), &objectTypeRef);

    // Define an anonymous type
    mdTypeDef newTypeDef;
    auto className = interception.Target.TypeName + interception.Target.MethodName + "Interception"_W;
    std::replace(className.begin(), className.end(), '.'_W, '_'_W);
    hr = metadataEmit->DefineTypeDef(className.c_str(), tdAbstract | tdSealed,
        objectTypeRef, NULL, &newTypeDef);

    // return type
    auto ret = target.signature.GetRet().GetRaw();
    

    auto xxx = target;
    auto yyy = GetFunctionInfo(metadataImport, targetMdToken);

    xxx.signature.TryParse();

    // number of arguments: original count //+ 2: mdToken + moduleVersionPtr
    auto numArguments = target.signature.NumberOfArguments();// +2;
    // and one more if it's an instance method
    if (target.signature.IsInstanceMethod())
    {
        numArguments += 1;
    }

    // create method sugnature
    std::vector<BYTE> signature{};
    //signature.push_back(callConvention);
    signature.push_back(numArguments);
    signature.insert(signature.end(), ret.begin(), ret.end());

    auto genericArgumentCount = 0;

    // insert type of this
    if (target.signature.IsInstanceMethod())
    {
        signature.push_back(ELEMENT_TYPE_OBJECT);
        /*auto instRaw = target.type.raw;
        if (instRaw.empty())
        {
            BYTE compressedToken[40];
            ULONG tokenLength = CorSigCompressToken(target.type.id, compressedToken);

            signature.push_back(ELEMENT_TYPE_CLASS);
            for (size_t i = 0; i < tokenLength; i++)
            {
                signature.push_back(compressedToken[i]);
            }
        }
        else
        {
            signature.insert(signature.end(), instRaw.begin(), instRaw.end());
        }

        if (target.type.IsGeneric())
        {
            genericArgumentCount += 1;
        }*/
    }

    // insert existing arguments
    auto args = target.signature.GetMethodArguments();
    
    for (const auto& arg : args)
    {
        signature.push_back(ELEMENT_TYPE_OBJECT);
        //auto raw = arg.GetRaw();
        //

        //if (arg.IsGeneric())
        //{
        //    genericArgumentCount += 1;
        //    //callConvention = CorCallingConvention(callConvention | IMAGE_CEE_CS_CALLCONV_GENERIC);

        //    signature.push_back(ELEMENT_TYPE_CLASS);
        //}

        //signature.insert(signature.end(), raw.begin(), raw.end());
    }

    // calling convention respecting generic
    CorCallingConvention callConvention = IMAGE_CEE_CS_CALLCONV_DEFAULT;
    /*if (target.isGeneric || genericArgumentCount > 0)
    {
        callConvention = CorCallingConvention(callConvention | IMAGE_CEE_CS_CALLCONV_GENERIC);
    }*/

    /*if (genericArgumentCount > 0)
    {
        signature.insert(signature.begin(), genericArgumentCount);
    }*/

    signature.insert(signature.begin(), callConvention);

    auto raw = target.signature.GetRaw();
    
    /*signature.push_back(ELEMENT_TYPE_I4);
    signature.push_back(ELEMENT_TYPE_I8);*/

    hr = metadataEmit->DefineMethod(newTypeDef,
        "__InterceptionMethod__"_W.c_str(),
        mdStatic | mdPublic /*| miAggressiveInlining*/,
        signature.data(),
        signature.size(),
        0,
        0,
        retMethodToken);

    // define wrapper.dll
    mdModuleRef wrapperRef;
    GetWrapperRef(hr, metadataAssemblyEmit, wrapperRef, interception.Interceptor.AssemblyName);
    IfFailRet(hr);

    // define wrappedType
    mdTypeRef wrapperTypeRef;
    hr = metadataEmit->DefineTypeRefByName(
        wrapperRef,
        interception.Interceptor.TypeName.data(),
        &wrapperTypeRef);
    IfFailRet(hr);

    // method
    mdMemberRef wrapperMethodRef;
    hr = metadataEmit->DefineMemberRef(
        wrapperTypeRef, interception.Interceptor.MethodName.c_str(),
        interception.Interceptor.Signature.data(),
        (DWORD)(interception.Interceptor.Signature.size()),
        &wrapperMethodRef);

    // local sig
    mdSignature localSigToken;
    COR_SIGNATURE localSig[/*11*/] = {
        IMAGE_CEE_CS_CALLCONV_LOCAL_SIG,
        0,
    };

    hr = metadataEmit->GetTokenFromSig(localSig, sizeof(localSig),
        &localSigToken);

    ILRewriter rewriter(this->corProfilerInfo, nullptr, moduleId, *retMethodToken);
    rewriter.InitializeTiny();
    rewriter.SetTkLocalVarSig(localSigToken);
    ILRewriterHelper helper(&rewriter);
    helper.SetILPosition(rewriter.GetILList()->m_pNext);

    for (size_t i = 0; i < numArguments; i++)
    {
        helper.LoadArgument(i);
    }

    helper.LoadInt32(targetMdToken);

    helper.LoadInt64(reinterpret_cast<INT64>(&modules[moduleId]));


    helper.CallMember(wrapperMethodRef, false);

    // ret
    helper.Ret();

    hr = rewriter.Export(false);

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

void CorProfiler::AddInterception(ImportInterception interception)
{
    configuration.interceptions.push_back(Interception(
        ToWSTRING(interception.CallerAssembly),
        TargetMethod(
            ToWSTRING(interception.TargetAssemblyName),
            ToWSTRING(interception.TargetTypeName),
            ToWSTRING(interception.TargetMethodName),
            interception.TargetMethodParametersCount),
        Interceptor(ToWSTRING(interception.InterceptorAssemblyName),
            ToWSTRING(interception.InterceptorTypeName),
            ToWSTRING(interception.InterceptorMethodName),
            std::vector<BYTE>(interception.Signature, interception.Signature + interception.SignatureLength))
    ));
}

#ifndef _WIN32
extern BYTE _binary_Interception_Loader_dll_start;
extern BYTE _binary_Interception_Loader_dll_end;
#endif

#include "resource.h"

void CorProfiler::GetAssemblyBytes(BYTE** assemblyArray, int* assemblySize)
{
#ifdef _WIN32
    HINSTANCE hInstance = DllHandle;
    auto dllLpName = MAKEINTRESOURCE(IDR_LOADER);
    HRSRC hResAssemblyInfo = FindResource(hInstance, dllLpName, L"LOADER");
    HGLOBAL hResAssembly = LoadResource(hInstance, hResAssemblyInfo);
    *assemblySize = SizeofResource(hInstance, hResAssemblyInfo);
    *assemblyArray = (LPBYTE)LockResource(hResAssembly);
#else
    *assemblyArray = &_binary_Interception_Loader_dll_start;
    *assemblySize = &_binary_Interception_Loader_dll_end - &_binary_Interception_Loader_dll_start;
#endif // _WIN32
}

#ifndef _WIN32
extern BYTE _binary_Interception_Loader_Class_Name_txt_start;
extern BYTE _binary_Interception_Loader_Class_Name_txt_end;
#endif

wstring CorProfiler::GetInterceptionLoaderClassName()
{
#ifdef _WIN32
    HINSTANCE hInstance = DllHandle;
    TCHAR profilerLoaderClass[160];
    LoadString(hInstance, PROFILER_LOADER_CLASS, profilerLoaderClass, sizeof(profilerLoaderClass) / sizeof(TCHAR));

    return wstring(profilerLoaderClass);
#else
    return Trim(ToWSTRING(std::string((char*)&_binary_Interception_Loader_Class_Name_txt_start)));
#endif // _WIN32
}
