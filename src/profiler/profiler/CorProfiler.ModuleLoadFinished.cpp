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
        std::pair<util::wstring, ModuleID> lm(moduleInfo.assembly.name, moduleId);
        loadedModules.insert(lm);
        return S_OK;
    }

    ComPtr<IUnknown> metadataInterfaces;
    hr = this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf());
    if (FAILED(hr))
    {
        logging::log(logging::LogLevel::ERR, "Failed ModuleLoadFinished GetModuleMetaData"_W);
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
        logging::log(logging::LogLevel::ERR, "Failed ModuleLoadFinished EnumTypeDefs"_W);
        return S_OK;
    }

    for (auto typeIndex = 0; typeIndex < typeDefsCount; typeIndex++) {
        const auto typeInfo = info::TypeInfo::GetTypeInfo(metadataImport, typeDefs[typeIndex]);

        // get all methods
        HCORENUM hcorenumMethods = 0;
        mdMethodDef methods[MAX_CLASS_NAME]{};
        ULONG methodsCount;
        hr = metadataImport->EnumMethods(&hcorenumMethods, typeDefs[typeIndex], methods, MAX_CLASS_NAME, &methodsCount);
        if (FAILED(hr)) {
            logging::log(logging::LogLevel::ERR, "Failed ModuleLoadFinished EnumInterfaceImpls"_W);
            return S_OK;
        }

        // get all implemented interfaces
        std::vector<info::TypeInfo> interfaceInfos = GetAllImplementedInterfaces(typeInfo, metadataImport);
        
        for (auto methodIndex = 0; methodIndex < methodsCount; methodIndex++) {
            auto functionInfo = info::FunctionInfo::GetFunctionInfo(metadataImport, methods[methodIndex]);
            functionInfo.Signature.ParseArguments();

            // find an interceptor either for this type
            // or for a base type
            auto interceptors = FindInterceptors(typeInfo, functionInfo);
            auto traces = FindTraces(typeInfo, functionInfo);
            // then find an interceptor for one implemented interfaces
            for (auto interfaceIndex = 0; interfaceIndex < interfaceInfos.size(); interfaceIndex++)
            {
                auto x = FindInterceptors(interfaceInfos[interfaceIndex], functionInfo);
                std::copy(
                    x.begin(),
                    x.end(),
                    std::back_inserter(interceptors));
            }

            if (!std::get<0>(traces))
            {
                for (auto interfaceIndex = 0; interfaceIndex < interfaceInfos.size(); interfaceIndex++)
                {
                    traces = FindTraces(interfaceInfos[interfaceIndex], functionInfo);
                    if (std::get<0>(traces))
                    {
                        break;
                    }
                }
            }

            auto offsets = FindOffsets(typeInfo, functionInfo);

            // no interceptor found => nothing to do
            if (interceptors.empty() && !std::get<0>(traces) && offsets.empty())
            {
                continue;
            }

            std::vector<configuration::StrictInterception> strict{};
            for (const auto& el : interceptors)
            {
                std::copy(el.Interceptors.begin(), el.Interceptors.end(), std::back_inserter(strict));
            }

            std::sort(strict.begin(), strict.end(), [](const configuration::StrictInterception& left, const configuration::StrictInterception& right) { return left.Priority < right.Priority; });

            ModuleID m1[1]{ moduleId };
            mdMethodDef m2[1]{ methods[methodIndex] };
            // save function info otherwise IMetaDataImport2 will throw an exception from Rejit-Handler
            // get first found interceptor
            auto ri = RejitInfo(moduleId, methods[methodIndex], functionInfo, strict, std::get<0>(traces), std::get<1>(traces), std::get<2>(traces), offsets);
            rejitInfo.push_back(ri);
            // and then request rejit
            hr = corProfilerInfo->RequestReJIT(1, m1, m2);

            logging::log(logging::LogLevel::INFO, "Rejit {0}.{1} with {2} interceptors"_W, ri.Info.Type.Name, ri.Info.Name, strict.size());
        }
    }

    logging::log(logging::LogLevel::VERBOSE, "Module {0} loaded and analyzed {1}"_W, moduleInfo.assembly.name, moduleId);

    return S_OK;
}