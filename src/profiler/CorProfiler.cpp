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
    loaderDllPath = GetEnvironmentValue("PROFILER_LOADER_DLL"_W);
    loaderClass = GetEnvironmentValue("PROFILER_LOADER_CLASS"_W);

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

    const auto pMetadataImport =
        metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);

    mdModule module;
    hr = pMetadataImport->GetModuleFromScope(&module);

    GUID module_version_id;
    hr = pMetadataImport->GetScopeProps(nullptr, 0, nullptr, &module_version_id);

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

        if (!loaderDllPath.empty())
        {
            return LoadAssemblyFromFile(
                moduleId,
                functionToken,
                functionId
            );
        }
        else
        {
            return LoadAssemblyFromResource(
                moduleId,
                functionToken,
                functionId
            );
        }
    }

    return Rewrite(moduleId, functionToken);
}

HRESULT CorProfiler::LoadAssemblyFromResource(ModuleID moduleId, mdMethodDef methodDef, FunctionID functionId)
{
    mdMethodDef ret_method_token;

    auto hr = GenerateVoidILStartupMethod(moduleId, &ret_method_token);

    ILRewriter rewriter(this->corProfilerInfo, nullptr, moduleId, methodDef);
    IfFailRet(rewriter.Import());

    ILRewriterHelper helper(&rewriter);
    helper.SetILPosition(rewriter.GetILList()->m_pNext);
    helper.CallMember(ret_method_token, false);
    hr = rewriter.Export(false);

    return S_OK;
}

HRESULT CorProfiler::GenerateVoidILStartupMethod(ModuleID moduleId,
    mdMethodDef* retMethodToken) {

    HRESULT hr;

    ComPtr<IUnknown> metadataInterfaces;
    IfFailRet(this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf()));

    const auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);
    const auto metadataEmit = metadataInterfaces.As<IMetaDataEmit2>(IID_IMetaDataEmit);
    const auto assemblyImport = metadataInterfaces.As<IMetaDataAssemblyImport>(IID_IMetaDataEmit);
    const auto metadataAssemblyEmit = metadataInterfaces.As<IMetaDataAssemblyEmit>(IID_IMetaDataAssemblyEmit);

    // define mscorlib.dll
    mdModuleRef mscorlibRef;
    GetMsCorLibRef(hr, metadataAssemblyEmit, mscorlibRef);
    IfFailRet(hr);

    // Define System.Object
    mdTypeRef objectTypeRef;
    metadataEmit->DefineTypeRefByName(mscorlibRef, SystemObject.data(), &objectTypeRef);

    // Define a new TypeDef __DDVoidMethodType__ that extends System.Object
    mdTypeDef newTypeDef;
    hr = metadataEmit->DefineTypeDef("__DDVoidMethodType__"_W.c_str(), tdAbstract | tdSealed,
        objectTypeRef, NULL, &newTypeDef);

    // Define a new static method __DDVoidMethodCall__ on the new type that has a void return type and takes no arguments
    BYTE initialize_signature[] = {
      IMAGE_CEE_CS_CALLCONV_DEFAULT, // Calling convention
      0,                             // Number of parameters
      ELEMENT_TYPE_VOID,             // Return type
    };
    hr = metadataEmit->DefineMethod(newTypeDef,
        "__DDVoidMethodCall__"_W.c_str(),
        mdStatic,
        initialize_signature,
        sizeof(initialize_signature),
        0,
        0,
        retMethodToken);

    // Define a method on the managed side that will PInvoke into the profiler method:
    // C++: void GetAssemblyBytes(BYTE** pAssemblyArray, int* assemblySize)
    // C#: static extern void GetAssemblyBytes(out IntPtr assemblyPtr, out int assemblySize)
    mdMethodDef pinvoke_method_def;
    COR_SIGNATURE get_assembly_bytes_signature[] = {
        IMAGE_CEE_CS_CALLCONV_DEFAULT, // Calling convention
        2,                             // Number of parameters
        ELEMENT_TYPE_VOID,             // Return type
        ELEMENT_TYPE_BYREF,            // List of parameter types
        ELEMENT_TYPE_I,
        ELEMENT_TYPE_BYREF,
        ELEMENT_TYPE_I4,
    };

    hr = metadataEmit->DefineMethod(
        newTypeDef, "GetAssemblyBytes"_W.c_str(), mdStatic | mdPinvokeImpl | mdHideBySig,
        get_assembly_bytes_signature, sizeof(get_assembly_bytes_signature), 0, 0,
        &pinvoke_method_def);

    metadataEmit->SetMethodImplFlags(pinvoke_method_def, miPreserveSig);

#ifdef _WIN32
    wstring native_profiler_file = "DotNetCoreProfiler.dll"_W;
#else // _WIN32
    wstring native_profiler_file = "DotNetCoreProfiler.so"_W;
#endif // _WIN32

    mdModuleRef profiler_ref;
    hr = metadataEmit->DefineModuleRef(native_profiler_file.c_str(),
        &profiler_ref);

    hr = metadataEmit->DefinePinvokeMap(pinvoke_method_def,
        0,
        "GetAssemblyBytes"_W.c_str(),
        profiler_ref);

    // Get a TypeRef for System.Byte
    mdTypeRef byte_type_ref;
    hr = metadataEmit->DefineTypeRefByName(mscorlibRef,
        SystemByte.data(),
        &byte_type_ref);

    // Get a TypeRef for System.Runtime.InteropServices.Marshal
    mdTypeRef marshal_type_ref;
    hr = metadataEmit->DefineTypeRefByName(mscorlibRef,
        SystemRuntimeInteropServicesMarshal.data(),
        &marshal_type_ref);

    // Get a MemberRef for System.Runtime.InteropServices.Marshal.Copy(IntPtr, Byte[], int, int)
    mdMemberRef marshal_copy_member_ref;
    COR_SIGNATURE marshal_copy_signature[] = {
        IMAGE_CEE_CS_CALLCONV_DEFAULT, // Calling convention
        4,                             // Number of parameters
        ELEMENT_TYPE_VOID,             // Return type
        ELEMENT_TYPE_I,                // List of parameter types
        ELEMENT_TYPE_SZARRAY,
        ELEMENT_TYPE_U1,
        ELEMENT_TYPE_I4,
        ELEMENT_TYPE_I4
    };
    hr = metadataEmit->DefineMemberRef(
        marshal_type_ref, Copy.data(), marshal_copy_signature,
        sizeof(marshal_copy_signature), &marshal_copy_member_ref);

    // Get a TypeRef for System.Reflection.Assembly
    mdTypeRef system_reflection_assembly_type_ref;
    hr = metadataEmit->DefineTypeRefByName(mscorlibRef,
        SystemReflectionAssembly.data(),
        &system_reflection_assembly_type_ref);

    // Get a TypeRef for System.AppDomain
    mdTypeRef system_appdomain_type_ref;
    hr = metadataEmit->DefineTypeRefByName(mscorlibRef,
        SystemAppDomain.data(),
        &system_appdomain_type_ref);

    // Get a MemberRef for System.AppDomain.get_CurrentDomain()
    // and System.AppDomain.Assembly.Load(byte[], byte[])

    // Create method signature for AppDomain.CurrentDomain property
    COR_SIGNATURE appdomain_get_current_domain_signature_start[] = {
        IMAGE_CEE_CS_CALLCONV_DEFAULT,
        0,
        ELEMENT_TYPE_CLASS, // ret = System.AppDomain
        // insert compressed token for System.AppDomain TypeRef here
    };
    ULONG start_length = sizeof(appdomain_get_current_domain_signature_start);

    BYTE system_appdomain_type_ref_compressed_token[4];
    ULONG token_length = CorSigCompressToken(system_appdomain_type_ref, system_appdomain_type_ref_compressed_token);

    COR_SIGNATURE* appdomain_get_current_domain_signature = new COR_SIGNATURE[start_length + token_length];
    memcpy(appdomain_get_current_domain_signature,
        appdomain_get_current_domain_signature_start,
        start_length);
    memcpy(&appdomain_get_current_domain_signature[start_length],
        system_appdomain_type_ref_compressed_token,
        token_length);

    mdMemberRef appdomain_get_current_domain_member_ref;
    hr = metadataEmit->DefineMemberRef(
        system_appdomain_type_ref,
        get_CurrentDomain.data(),
        appdomain_get_current_domain_signature,
        start_length + token_length,
        &appdomain_get_current_domain_member_ref);
    delete[] appdomain_get_current_domain_signature;

    // Create method signature for AppDomain.Load(byte[], byte[])
    COR_SIGNATURE appdomain_load_signature_start[] = {
        IMAGE_CEE_CS_CALLCONV_HASTHIS,
        1,
        ELEMENT_TYPE_CLASS  // ret = System.Reflection.Assembly
        // insert compressed token for System.Reflection.Assembly TypeRef here
    };
    COR_SIGNATURE appdomain_load_signature_end[] = {
        ELEMENT_TYPE_SZARRAY,
        ELEMENT_TYPE_U1
    };
    start_length = sizeof(appdomain_load_signature_start);
    ULONG end_length = sizeof(appdomain_load_signature_end);

    BYTE system_reflection_assembly_type_ref_compressed_token[4];
    token_length = CorSigCompressToken(system_reflection_assembly_type_ref, system_reflection_assembly_type_ref_compressed_token);

    COR_SIGNATURE* appdomain_load_signature = new COR_SIGNATURE[start_length + token_length + end_length];
    memcpy(appdomain_load_signature,
        appdomain_load_signature_start,
        start_length);
    memcpy(&appdomain_load_signature[start_length],
        system_reflection_assembly_type_ref_compressed_token,
        token_length);
    memcpy(&appdomain_load_signature[start_length + token_length],
        appdomain_load_signature_end,
        end_length);

    mdMemberRef appdomain_load_member_ref;
    hr = metadataEmit->DefineMemberRef(
        system_appdomain_type_ref, Load.data(),
        appdomain_load_signature,
        start_length + token_length + end_length,
        &appdomain_load_member_ref);
    delete[] appdomain_load_signature;

    // Create method signature for Assembly.CreateInstance(string)
    COR_SIGNATURE assembly_create_instance_signature[] = {
        IMAGE_CEE_CS_CALLCONV_HASTHIS,
        1,
        ELEMENT_TYPE_OBJECT,  // ret = System.Object
        ELEMENT_TYPE_STRING
    };

    mdMemberRef assembly_create_instance_member_ref;
    hr = metadataEmit->DefineMemberRef(
        system_reflection_assembly_type_ref, CreateInstance.data(),
        assembly_create_instance_signature,
        sizeof(assembly_create_instance_signature),
        &assembly_create_instance_member_ref);

    auto load_helper_str = "Interception.Loader"_W;

    mdString load_helper_token;
    hr = metadataEmit->DefineUserString(load_helper_str.c_str(), (ULONG)load_helper_str.length(),
        &load_helper_token);

    ULONG string_len = 0;
    WCHAR string_contents[NameMaxSize]{};
    hr = metadataImport->GetUserString(load_helper_token, string_contents,
        NameMaxSize, &string_len);

    // Generate a locals signature defined in the following way:
    //   [0] System.IntPtr ("assemblyPtr" - address of assembly bytes)
    //   [1] System.Int32  ("assemblySize" - size of assembly bytes)
    //   [2] System.Byte[] ("assemblyBytes" - managed byte array for assembly)
    //   [3] class System.Reflection.Assembly ("loadedAssembly" - assembly instance to save loaded assembly)
    mdSignature locals_signature_token;
    COR_SIGNATURE locals_signature[6] = {
        IMAGE_CEE_CS_CALLCONV_LOCAL_SIG, // Calling convention
        3,                               // Number of variables
        ELEMENT_TYPE_I,                  // List of variable types
        ELEMENT_TYPE_I4,
        ELEMENT_TYPE_SZARRAY,
        ELEMENT_TYPE_U1
    };

    hr = metadataEmit->GetTokenFromSig(locals_signature, sizeof(locals_signature),
        &locals_signature_token);

    /////////////////////////////////////////////
    // Add IL instructions into the void method
    ILRewriter rewriter_void(this->corProfilerInfo, nullptr, moduleId, *retMethodToken);
    rewriter_void.InitializeTiny();
    rewriter_void.SetTkLocalVarSig(locals_signature_token);
    ILInstr* pFirstInstr = rewriter_void.GetILList()->m_pNext;
    ILInstr* pNewInstr = NULL;

    // Step 1) Call void GetAssemblyBytes(out IntPtr assemblyPtr, out int assemblySize)

    // ldloca.s 0 : Load the address of the "assemblyPtr" variable (locals index 0)
    pNewInstr = rewriter_void.NewILInstr();
    pNewInstr->m_opcode = CEE_LDLOCA_S;
    pNewInstr->m_Arg32 = 0;
    rewriter_void.InsertBefore(pFirstInstr, pNewInstr);

    // ldloca.s 1 : Load the address of the "assemblySize" variable (locals index 1)
    pNewInstr = rewriter_void.NewILInstr();
    pNewInstr->m_opcode = CEE_LDLOCA_S;
    pNewInstr->m_Arg32 = 1;
    rewriter_void.InsertBefore(pFirstInstr, pNewInstr);

    // call void GetAssemblyBytes(out IntPtr assemblyPtr, out int assemblySize)
    pNewInstr = rewriter_void.NewILInstr();
    pNewInstr->m_opcode = CEE_CALL;
    pNewInstr->m_Arg32 = pinvoke_method_def;
    rewriter_void.InsertBefore(pFirstInstr, pNewInstr);

    // Step 2) Call void Marshal.Copy(IntPtr source, byte[] destination, int startIndex, int length) to populate the managed assembly bytes

    // ldloc.1 : Load the "assemblySize" variable (locals index 1)
    pNewInstr = rewriter_void.NewILInstr();
    pNewInstr->m_opcode = CEE_LDLOC_1;
    rewriter_void.InsertBefore(pFirstInstr, pNewInstr);

    // newarr System.Byte : Create a new Byte[] to hold a managed copy of the assembly data
    pNewInstr = rewriter_void.NewILInstr();
    pNewInstr->m_opcode = CEE_NEWARR;
    pNewInstr->m_Arg32 = byte_type_ref;
    rewriter_void.InsertBefore(pFirstInstr, pNewInstr);

    // stloc.s 4 : Assign the Byte[] to the "assemblyBytes" variable (locals index 4)
    pNewInstr = rewriter_void.NewILInstr();
    pNewInstr->m_opcode = CEE_STLOC_S;
    pNewInstr->m_Arg8 = 2;
    rewriter_void.InsertBefore(pFirstInstr, pNewInstr);

    // ldloc.0 : Load the "assemblyPtr" variable (locals index 0)
    pNewInstr = rewriter_void.NewILInstr();
    pNewInstr->m_opcode = CEE_LDLOC_0;
    rewriter_void.InsertBefore(pFirstInstr, pNewInstr);

    // ldloc.s 4 : Load the "assemblyBytes" variable (locals index 4)
    pNewInstr = rewriter_void.NewILInstr();
    pNewInstr->m_opcode = CEE_LDLOC_S;
    pNewInstr->m_Arg8 = 2;
    rewriter_void.InsertBefore(pFirstInstr, pNewInstr);

    // ldc.i4.0 : Load the integer 0 for the Marshal.Copy startIndex parameter
    pNewInstr = rewriter_void.NewILInstr();
    pNewInstr->m_opcode = CEE_LDC_I4_0;
    rewriter_void.InsertBefore(pFirstInstr, pNewInstr);

    // ldloc.1 : Load the "assemblySize" variable (locals index 1) for the Marshal.Copy length parameter
    pNewInstr = rewriter_void.NewILInstr();
    pNewInstr->m_opcode = CEE_LDLOC_1;
    rewriter_void.InsertBefore(pFirstInstr, pNewInstr);

    // call Marshal.Copy(IntPtr source, byte[] destination, int startIndex, int length)
    pNewInstr = rewriter_void.NewILInstr();
    pNewInstr->m_opcode = CEE_CALL;
    pNewInstr->m_Arg32 = marshal_copy_member_ref;
    rewriter_void.InsertBefore(pFirstInstr, pNewInstr);

    // Step 4) Call System.Reflection.Assembly System.AppDomain.CurrentDomain.Load(byte[]))

    // call System.AppDomain System.AppDomain.CurrentDomain property
    pNewInstr = rewriter_void.NewILInstr();
    pNewInstr->m_opcode = CEE_CALL;
    pNewInstr->m_Arg32 = appdomain_get_current_domain_member_ref;
    rewriter_void.InsertBefore(pFirstInstr, pNewInstr);

    // ldloc.s 4 : Load the "assemblyBytes" variable (locals index 2) for the first byte[] parameter of AppDomain.Load(byte[], byte[])
    pNewInstr = rewriter_void.NewILInstr();
    pNewInstr->m_opcode = CEE_LDLOC_S;
    pNewInstr->m_Arg8 = 2;
    rewriter_void.InsertBefore(pFirstInstr, pNewInstr);

    // callvirt System.Reflection.Assembly System.AppDomain.Load(uint8[])
    pNewInstr = rewriter_void.NewILInstr();
    pNewInstr->m_opcode = CEE_CALLVIRT;
    pNewInstr->m_Arg32 = appdomain_load_member_ref;
    rewriter_void.InsertBefore(pFirstInstr, pNewInstr);

    //// stloc.s 6 : Assign the System.Reflection.Assembly object to the "loadedAssembly" variable (locals index 3
    //pNewInstr = rewriter_void.NewILInstr();
    //pNewInstr->m_opcode = CEE_STLOC_S;
    //pNewInstr->m_Arg8 = 3;
    //rewriter_void.InsertBefore(pFirstInstr, pNewInstr);

    //// Step 4) Call instance method Assembly.CreateInstance("Datadog.Trace.ClrProfiler.Managed.Loader.Startup")

    //// ldloc.s 6 : Load the "loadedAssembly" variable (locals index 6) to call Assembly.CreateInstance
    //pNewInstr = rewriter_void.NewILInstr();
    //pNewInstr->m_opcode = CEE_LDLOC_S;
    //pNewInstr->m_Arg8 = 3;
    //rewriter_void.InsertBefore(pFirstInstr, pNewInstr);

    // ldstr "Datadog.Trace.ClrProfiler.Managed.Loader.Startup"
    pNewInstr = rewriter_void.NewILInstr();
    pNewInstr->m_opcode = CEE_LDSTR;
    pNewInstr->m_Arg32 = load_helper_token;
    rewriter_void.InsertBefore(pFirstInstr, pNewInstr);

    // callvirt System.Object System.Reflection.Assembly.CreateInstance(string)
    pNewInstr = rewriter_void.NewILInstr();
    pNewInstr->m_opcode = CEE_CALLVIRT;
    pNewInstr->m_Arg32 = assembly_create_instance_member_ref;
    rewriter_void.InsertBefore(pFirstInstr, pNewInstr);

    // pop the returned object
    pNewInstr = rewriter_void.NewILInstr();
    pNewInstr->m_opcode = CEE_POP;
    rewriter_void.InsertBefore(pFirstInstr, pNewInstr);

    // return
    pNewInstr = rewriter_void.NewILInstr();
    pNewInstr->m_opcode = CEE_RET;
    rewriter_void.InsertBefore(pFirstInstr, pNewInstr);

    hr = rewriter_void.Export(false);

    return S_OK;
}

HRESULT CorProfiler::LoadAssemblyFromFile(
    ModuleID moduleId,
    mdMethodDef methodDef,
    FunctionID functionId)
{
    HRESULT hr;

    ComPtr<IUnknown> metadataInterfaces;
    IfFailRet(this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf()));

    const auto metadataEmit = metadataInterfaces.As<IMetaDataEmit2>(IID_IMetaDataEmit);

    const auto metadataAssemblyEmit = metadataInterfaces.As<IMetaDataAssemblyEmit>(IID_IMetaDataAssemblyEmit);

    const auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);

    ILRewriter rewriter(this->corProfilerInfo, nullptr, moduleId, methodDef);

    IfFailRet(rewriter.Import());

    mdString aPath;
    hr = metadataEmit->DefineUserString(loaderDllPath.c_str(), (ULONG)loaderDllPath.length(),
        &aPath);

    ULONG string_len = 0;
    WCHAR string_contents[NameMaxSize]{};
    hr = metadataImport->GetUserString(aPath, string_contents,
        NameMaxSize, &string_len);
    IfFailRet(hr);

    // define mscorlib.dll
    mdModuleRef mscorlibRef;
    GetMsCorLibRef(hr, metadataAssemblyEmit, mscorlibRef);
    IfFailRet(hr);

    // define type System.Reflection.Assembly
    mdTypeRef assemblyTypeRef;
    hr = metadataEmit->DefineTypeRefByName(
        mscorlibRef,
        SystemReflectionAssembly.c_str(),
        &assemblyTypeRef);

    unsigned buffer;
    auto size = CorSigCompressToken(assemblyTypeRef, &buffer);
    auto* assemblyLoadSignature = new COR_SIGNATURE[size + 4];
    unsigned offset = 0;
    assemblyLoadSignature[offset++] = IMAGE_CEE_CS_CALLCONV_DEFAULT;
    assemblyLoadSignature[offset++] = 0x01;
    assemblyLoadSignature[offset++] = ELEMENT_TYPE_CLASS;
    memcpy(&assemblyLoadSignature[offset], &buffer, size);
    offset += size;
    assemblyLoadSignature[offset] = ELEMENT_TYPE_STRING;

    // define method System.Reflection.Assembly.LoadFrom
    mdMemberRef assemblyLoadMemberRef;
    hr = metadataEmit->DefineMemberRef(
        assemblyTypeRef,
        LoadFrom.data(),
        assemblyLoadSignature,
        sizeof(assemblyLoadSignature),
        &assemblyLoadMemberRef);

    // Create method signature for Assembly.CreateInstance(string)
    COR_SIGNATURE createInstanceSignature[] = {
        IMAGE_CEE_CS_CALLCONV_HASTHIS,
        1,
        ELEMENT_TYPE_OBJECT,
        ELEMENT_TYPE_STRING
    };
    mdMemberRef createInstanceMemberRef;
    hr = metadataEmit->DefineMemberRef(
        assemblyTypeRef, CreateInstance.c_str(),
        createInstanceSignature,
        sizeof(createInstanceSignature),
        &createInstanceMemberRef);

    // define path to a .net dll 
    mdString profilerLoaderDllNameTextToken;
    hr = metadataEmit->DefineUserString(loaderDllPath.data(), (ULONG)loaderDllPath.length(), &profilerLoaderDllNameTextToken);

    std::cout << "AssemblyLoader " << ToString(loaderDllPath) << std::endl;

    ILRewriterHelper helper(&rewriter);
    helper.SetILPosition(rewriter.GetILList()->m_pNext);

    // load assembly path
    helper.LoadStr(profilerLoaderDllNameTextToken);
    // load assembly
    helper.CallMember(assemblyLoadMemberRef, false);

    mdString initializerTypeToken;
    hr = metadataEmit->DefineUserString(loaderClass.data(), (ULONG)loaderClass.length(), &initializerTypeToken);

    // load initializer type name
    helper.LoadStr(initializerTypeToken);
    // create an instance of the initializer
    helper.CallMember(createInstanceMemberRef, true);

    // pop result as not needed
    helper.Pop();

    IfFailRet(rewriter.Export(false));

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

HRESULT CorProfiler::Rewrite(const ModuleID& moduleId, const mdToken& callerToken)
{
    HRESULT hr;

    ILRewriter rewriter(this->corProfilerInfo, nullptr, moduleId, callerToken);
    IfFailRet(rewriter.Import());

    ComPtr<IUnknown> metadataInterfaces;
    IfFailRet(this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf()));

    const auto pMetadataEmit = metadataInterfaces.As<IMetaDataEmit2>(IID_IMetaDataEmit);

    const auto pMetadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);

    const auto pMetadataAssemblyEmit = metadataInterfaces.As<IMetaDataAssemblyEmit>(IID_IMetaDataAssemblyEmit);

    auto functionInfo = GetFunctionInfo(pMetadataImport, callerToken);
    hr = functionInfo.signature.TryParse();

    auto moduleInfo = GetModuleInfo(this->corProfilerInfo, moduleId);

    if (!SkipAssembly(moduleInfo.assembly.name)) for (ILInstr* pInstr = rewriter.GetILList()->m_pNext;
        pInstr != rewriter.GetILList(); pInstr = pInstr->m_pNext) {
        if (pInstr->m_opcode != CEE_CALL && pInstr->m_opcode != CEE_CALLVIRT) {
            continue;
        }

        auto target =
            GetFunctionInfo(pMetadataImport, pInstr->m_Arg32);
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
                auto m = modules[moduleId];

                std::cout << "Found call to " << ToString(target.type.name) << "." << ToString(target.name)
                    << " num args " << target.signature.NumberOfArguments()
                    << " from assembly " << ToString(moduleInfo.assembly.name)
                    << " module " << m
                    << std::endl << std::flush;

                // define wrapper.dll
                mdModuleRef wrapperRef;
                GetWrapperRef(hr, pMetadataAssemblyEmit, wrapperRef, interception.Interceptor.AssemblyName);
                IfFailRet(hr);

                // define wrappedType
                mdTypeRef wrapperTypeRef;
                hr = pMetadataEmit->DefineTypeRefByName(
                    wrapperRef,
                    interception.Interceptor.TypeName.data(),
                    &wrapperTypeRef);
                IfFailRet(hr);

                // method
                mdMemberRef wrapperMethodRef;
                hr = pMetadataEmit->DefineMemberRef(
                    wrapperTypeRef, interception.Interceptor.MethodName.c_str(),
                    interception.Interceptor.Signature.data(),
                    (DWORD)(interception.Interceptor.Signature.size()),
                    &wrapperMethodRef);

                ILRewriterHelper helper(&rewriter);
                helper.SetILPosition(pInstr);
                helper.LoadInt32(targetMdToken);

                const void* module_version_id_ptr = &modules[moduleId];

                helper.LoadInt64(reinterpret_cast<INT64>(module_version_id_ptr));

                helper.CallMember(wrapperMethodRef, false);

                pInstr->m_opcode = CEE_NOP;
                            
                IfFailRet(rewriter.Export(false));
            }
        }
    }

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

void CorProfiler::GetAssemblyBytes(BYTE** assemblyArray, int* assemblySize)
{
#ifdef _WIN32
    throw std::exception("Not implemented");
#else
    *assemblyArray = &_binary_Interception_Loader_dll_start;
    *assemblySize = &_binary_Interception_Loader_dll_end - &_binary_Interception_Loader_dll_start;
#endif // _WIN32
}
