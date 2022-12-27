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

bool CorProfiler::SkipAssembly(const wstring& name)
{
    return configuration.SkipAssemblies.find(name) != configuration.SkipAssemblies.end();
}

HRESULT STDMETHODCALLTYPE CorProfiler::ModuleLoadFinished(ModuleID moduleId, HRESULT hrStatus)
{
    std::lock_guard<std::mutex> guard(mutex);

    HRESULT hr;
    const auto moduleInfo = info::ModuleInfo::GetModuleInfo(this->corProfilerInfo, moduleId);

    if (SkipAssembly(moduleInfo.assembly.name)) {
        skippedModules.insert(moduleId);
    }

    ComPtr<IUnknown> metadataInterfaces;
    hr = this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf());
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed ModuleLoadFinished GetModuleMetaData"_W);
        return hr;
    }

    auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);

    GUID module_version_id;
    hr = metadataImport->GetScopeProps(nullptr, 0, nullptr, &module_version_id);

    logging::log(logging::LogLevel::VERBOSE, "Module {0} loaded {1}"_W, moduleInfo.assembly.name, moduleId);

    std::pair<util::wstring, ModuleID> lm(moduleInfo.assembly.name, moduleId);
    loadedModules.insert(lm);

    if (moduleInfo.assembly.name == "interceptor"_W) {
        std::cout << "interceptor" << std::endl;
        HCORENUM hcorenum = 0;
        const auto maxMethods = 1000;
        mdTypeDef types[maxMethods]{};
        ULONG cnt;
        hr = metadataImport->EnumTypeDefs(&hcorenum, types, maxMethods, &cnt);
        if (FAILED(hr)) {
            std::cout << "failed EnumTypeDefs" << std::endl;
        }
        else {
            std::cout << "EnumTypeDefs " << cnt << std::endl;
            for (auto i = 0; i < cnt; i++) {
                const auto typeInfo = info::TypeInfo::GetTypeInfo(metadataImport, types[i]);

                std::cout << "EnumTypeDefs " << util::ToString(typeInfo.Name) << std::endl;

                if (typeInfo.Name == "interceptor.M2"_W || typeInfo.Name == "interceptor.DefaultInitializer"_W) {
                    HCORENUM hcorenum = 0;
                    const auto maxMethods = 1000;
                    mdMethodDef methods[maxMethods]{};
                    ULONG cnt2;
                    metadataImport->EnumMethods(&hcorenum, types[i], methods, maxMethods, &cnt2);
                    if (FAILED(hr)) {
                        std::cout << "failed EnumMethods" << std::endl;
                    }
                    else
                    {
                        for (auto k = 0; k < cnt2; k++) {
                            const auto functionInfo = info::FunctionInfo::GetFunctionInfo(metadataImport, methods[k]);
                            std::cout << "EnumMethods " << util::ToString(functionInfo.Name) << std::endl;

                            for (auto j = 0; j < functionInfo.Signature.Raw.size(); j++)
                            {
                                std::cout << std::hex << (int)functionInfo.Signature.Raw[j] << std::endl;
                            }

                            std::cout << "EnumMethods " << util::ToString(functionInfo.Name) << std::endl;
                        }
                    }

                    

                    //break;
                }
            }
        }
    }

    // check all strict interceptors
    for (const auto& interceptor : configuration.StrictInterceptions) {
        // for this assembly
        if (interceptor.Target.AssemblyName == moduleInfo.assembly.name) {
            // by type
            mdTypeDef typeDef = mdTypeRefNil;
            metadataImport->FindTypeDefByName(interceptor.Target.TypeName.c_str(), mdTokenNil, &typeDef);

            if (typeDef == mdTypeRefNil) {
                logging::log(logging::LogLevel::VERBOSE,
                    "No type {0} found in assembly {1}, skip"_W, interceptor.Target.TypeName, moduleInfo.assembly.name);

                continue;
            }

            HCORENUM hcorenum = 0;
            const auto maxMethods = 1000;
            mdMethodDef methods[maxMethods]{};
            ULONG cnt;
            metadataImport->EnumMethods(&hcorenum, typeDef, methods, maxMethods, &cnt);
            // TODO interfaces
            // TODO trace attributes
            // TODO dump local variables based on PDB
            for (auto i = 0; i < cnt; i++) {
                const auto functionInfo = info::FunctionInfo::GetFunctionInfo(metadataImport, methods[i]);

                // then by name and by argument count
                // TODO check arguments and return type 
                if (functionInfo.Name == interceptor.Target.MethodName &&
                    functionInfo.Signature.NumberOfArguments() == interceptor.Target.MethodParametersCount) {
                    logging::log(logging::LogLevel::INFO,
                        "Found an interceptor for {0}.{1}.{2} with {3} arguments"_W, interceptor.Target.AssemblyName, interceptor.Target.TypeName, interceptor.Target.MethodName, interceptor.Target.MethodParametersCount);

                    logging::log(logging::LogLevel::VERBOSE,
                        "Found an interceptor for {0}.{1}.{2} with {3} arguments moduleId {4} functionId {5}"_W,
                        interceptor.Target.AssemblyName,
                        interceptor.Target.TypeName,
                        interceptor.Target.MethodName,
                        interceptor.Target.MethodParametersCount,
                        moduleId,
                        methods[i]);

                    ModuleID m1[1]{ moduleId };
                    mdMethodDef m2[1]{ methods[i] };
                    // save function info otherwise IMetaDataImport2 will throw an exception from Rejit-Handler
                    rejitInfo.push_back(RejitInfo(moduleId, methods[i], functionInfo, interceptor));
                    // and then request rejit
                    hr = corProfilerInfo->RequestReJIT(1, m1, m2);
                }
            }
        }
    }

    return S_OK;
}
