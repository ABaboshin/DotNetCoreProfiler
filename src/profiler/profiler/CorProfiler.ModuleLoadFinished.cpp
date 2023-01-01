#include <algorithm>
#include <iostream>
#include <string>
#include "corhlpr.h"
#include "corhdr.h"
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
        return S_OK;
    }

    ComPtr<IUnknown> metadataInterfaces;
    hr = this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf());
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed ModuleLoadFinished GetModuleMetaData"_W);
        return S_OK;
    }

    auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);

    logging::log(logging::LogLevel::VERBOSE, "Module {0} loaded {1}"_W, moduleInfo.assembly.name, moduleId);

    std::pair<util::wstring, ModuleID> lm(moduleInfo.assembly.name, moduleId);
    loadedModules.insert(lm);

    HCORENUM hcorenumTypeDefs = 0;
    mdTypeDef typeDefs[MAX_CLASS_NAME]{};
    ULONG typeDefsCount;
    hr = metadataImport->EnumTypeDefs(&hcorenumTypeDefs, typeDefs, MAX_CLASS_NAME, &typeDefsCount);
    if (FAILED(hr)) {
        logging::log(logging::LogLevel::NONSUCCESS, "Failed ModuleLoadFinished EnumTypeDefs"_W);
        return S_OK;
    }

    // TODO trace attributes
    // TODO dump local variables based on PDB
    // iterate over all types in the module
    for (auto typeIndex = 0; typeIndex < typeDefsCount; typeIndex++) {
        const auto typeInfo = info::TypeInfo::GetTypeInfo(metadataImport, typeDefs[typeIndex]);

        // get all methods
        HCORENUM hcorenumMethods = 0;
        mdMethodDef methods[MAX_CLASS_NAME]{};
        ULONG methodsCount;
        hr = metadataImport->EnumMethods(&hcorenumMethods, typeDefs[typeIndex], methods, MAX_CLASS_NAME, &methodsCount);
        if (FAILED(hr)) {
            logging::log(logging::LogLevel::NONSUCCESS, "Failed ModuleLoadFinished EnumInterfaceImpls"_W);
            return S_OK;
        }

        // get all implemented interfaces
        std::vector<info::TypeInfo> interfaceInfos = GetAllImplementedInterfaces(typeInfo, metadataImport);
        
        for (auto methodIndex = 0; methodIndex < methodsCount; methodIndex++) {
            auto functionInfo = info::FunctionInfo::GetFunctionInfo(metadataImport, methods[methodIndex]);
            functionInfo.Signature.ParseArguments();

            // try firstly find an interceptor either for this type
            // or for a base type
            auto interceptors = FindInterceptors(typeInfo, functionInfo);
            // if not found try to find an interceptor for one implemented interfaces
            if (interceptors.empty())
            {
                for (auto interfaceIndex = 0; interfaceIndex < interfaceInfos.size(); interfaceIndex++)
                {
                    interceptors = FindInterceptors(interfaceInfos[interfaceIndex], functionInfo);
                    if (!interceptors.empty())
                    {
                        break;
                    }
                }
                
            }

            // no interceptor found => nothing to do
            if (interceptors.empty())
            {
                continue;
            }

            std::sort(interceptors.begin(), interceptors.end(), [](const configuration::StrictInterception& left, const configuration::StrictInterception& right) { return left.Priority < right.Priority; });

            ModuleID m1[1]{ moduleId };
            mdMethodDef m2[1]{ methods[methodIndex] };
            // save function info otherwise IMetaDataImport2 will throw an exception from Rejit-Handler
            // get first found interceptor
            auto ri = RejitInfo(moduleId, methods[methodIndex], functionInfo, interceptors);
            rejitInfo.push_back(ri);
            // and then request rejit
            hr = corProfilerInfo->RequestReJIT(1, m1, m2);

            logging::log(logging::LogLevel::INFO, "Rejit {0}.{1} with {2} interceptors"_W, ri.Info.Type.Name, ri.Info.Name, interceptors.size());
        }
    }

    logging::log(logging::LogLevel::VERBOSE, "Module {0} loaded and analyzed {1}"_W, moduleInfo.assembly.name, moduleId);

    return S_OK;
}