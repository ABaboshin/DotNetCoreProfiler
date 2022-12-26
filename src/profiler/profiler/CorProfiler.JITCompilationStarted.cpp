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

    hr = this->corProfilerInfo->GetFunctionInfo(functionId, &classId, &moduleId, &functionToken);
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed GetFunctionInfo"_W);
        return hr;
    }

    // if the current call is not a call to one of skipped assemblies
    if (skippedModules.find(moduleId) != skippedModules.end())
    {
        return S_OK;
    }

    auto moduleInfo = info::ModuleInfo::GetModuleInfo(this->corProfilerInfo, moduleId);

    // quick exit if already loaded into app domain
    if (loadedIntoAppDomains.find(moduleInfo.assembly.appDomainId) != loadedIntoAppDomains.end())
    {
        return S_OK;
    }

    auto rewriter = CreateILRewriter(nullptr, moduleId, functionToken);
    hr = rewriter->Import();
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed Import"_W);
        return hr;
    }

    // load once into appdomain
    loadedIntoAppDomains.insert(moduleInfo.assembly.appDomainId);

    ComPtr<IUnknown> metadataInterfaces;
    hr = this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf());
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed GetModuleMetaData"_W);
        return hr;
    }

    const auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);

    const auto functionInfo = info::FunctionInfo::GetFunctionInfo(metadataImport, functionToken);

    hr = InjectLoadMethod(moduleId, *rewriter, functionInfo);
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed InjectLoadMethod"_W);
        return hr;
    }


    hr = rewriter->Export();

    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed Export"_W);
        return hr;
    }

    if (oneAppDomainMode) {
        this->corProfilerInfo->SetEventMask(eventMask & ~COR_PRF_MONITOR_JIT_COMPILATION);
        if (FAILED(hr)) {
            logging::log(logging::LogLevel::NONSUCCESS, "Failed SetEventMask"_W);
            return hr;
        }
    }

    return S_OK;
}

HRESULT CorProfiler::InjectLoadMethod(ModuleID moduleId, rewriter::ILRewriter& rewriter, const info::FunctionInfo& functionInfo)
{
    mdMethodDef retMethodToken;

    auto hr = GenerateLoadMethod(moduleId, retMethodToken, functionInfo);
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed GenerateLoadMethod GetModuleMetaData"_W);
        return hr;
    }

    rewriter::ILRewriterHelper helper(&rewriter);
    helper.SetILPosition(rewriter.GetILList()->m_pNext);
    helper.CallMember(retMethodToken, false);

    ComPtr<IUnknown> metadataInterfaces;
    hr = this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf());
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed InjectLoadMethod GetModuleMetaData"_W);
        return hr;
    }

    const auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);

    logging::log(logging::LogLevel::VERBOSE, "{0}"_W, ilDumper.DumpILCodes("main ", &rewriter, functionInfo, metadataImport));

    return S_OK;
}

HRESULT CorProfiler::GenerateLoadMethod(ModuleID moduleId, mdMethodDef& retMethodToken, const info::FunctionInfo& functionInfo)
{
    HRESULT hr;

    ComPtr<IUnknown> metadataInterfaces;
    hr = this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf());
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed GenerateLoadMethod GetModuleMetaData"_W);
        return hr;
    }

    const auto metadataEmit = metadataInterfaces.As<IMetaDataEmit2>(IID_IMetaDataEmit);
    const auto metadataAssemblyEmit = metadataInterfaces.As<IMetaDataAssemblyEmit>(IID_IMetaDataAssemblyEmit);
    const auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);

    // define mscorlib.dll
    mdModuleRef mscorlibRef;
    hr = GetMsCorLibRef(metadataAssemblyEmit, mscorlibRef);
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed GenerateLoadMethod GetMsCorLibRef"_W);
        return hr;
    }

    // Define System.Object
    mdTypeRef objectTypeRef;
    metadataEmit->DefineTypeRefByName(mscorlibRef, _const::SystemObject.data(), &objectTypeRef);

    // Define an anonymous type
    mdTypeDef newTypeDef;
    hr = metadataEmit->DefineTypeDef("__Profiler__"_W.c_str(), tdAbstract | tdSealed,
        objectTypeRef, NULL, &newTypeDef);

    // Define a new static method
    COR_SIGNATURE loadMethodSignature[] = {
        IMAGE_CEE_CS_CALLCONV_DEFAULT,
        0,
        ELEMENT_TYPE_VOID };

    hr = metadataEmit->DefineMethod(newTypeDef,
        "__Loader__"_W.c_str(),
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

    rewriter::ILRewriterHelper helper(rewriter);
    helper.SetILPosition(rewriter->GetILList()->m_pNext);

    {
        logging::log(logging::LogLevel::INFO, "Load {0}"_W, configuration.Loader.AssemblyPath);

        // assembly path
        mdString pathToken;
        hr = metadataEmit->DefineUserString(configuration.Loader.AssemblyPath.c_str(), (ULONG)configuration.Loader.AssemblyPath.length(), &pathToken);

        helper.LoadStr(pathToken);
        helper.CallMember(assemblyLoadFromRef, false);

        logging::log(logging::LogLevel::INFO, "Loader {0} from {1}"_W, configuration.Loader.TypeName, configuration.Loader.AssemblyPath);


        mdString loaderToken;
        metadataEmit->DefineUserString(configuration.Loader.TypeName.c_str(), (ULONG)configuration.Loader.TypeName.length(), &loaderToken);

        helper.LoadStr(loaderToken);
        helper.CallMember(createInstanceMemberRef, true);
        helper.Pop();

        // ret
        helper.Ret();

        logging::log(logging::LogLevel::VERBOSE, "{0}"_W, ilDumper.DumpILCodes("load ", rewriter, functionInfo, metadataImport));

        hr = rewriter->Export();

        if (FAILED(hr))
        {
            logging::log(logging::LogLevel::INFO, "Failed to create load method"_W);
        }

        return hr;
    }
}
