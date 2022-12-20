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
    IfFailRet(this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf()));

    auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);

    /*mdModule module;
    hr = metadataImport->GetModuleFromScope(&module);*/

    GUID module_version_id;
    hr = metadataImport->GetScopeProps(nullptr, 0, nullptr, &module_version_id);

    modules[moduleId] = module_version_id;

    logging::log(logging::LogLevel::VERBOSE,
        "Module {0} loaded"_W, moduleInfo.assembly.name);

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
                    rejitInfo.push_back(RejitInfo(moduleId, methods[i], functionInfo));
                    // and then request rejit
                    hr = corProfilerInfo->RequestReJIT(1, m1, m2);
                }
            }
        }
    }

    //if (moduleInfo.assembly.name == "app"_W) {
    //  logging::log(logging::LogLevel::INFO,
    //               "Module {0} analyze"_W, moduleInfo.assembly.name);
    //  // enabledModules.push_back(moduleId);

    //  mdTypeDef typeDef = mdTypeRefNil;
    //  metadataImport->FindTypeDefByName("app.Program"_W.c_str(), mdTokenNil, &typeDef);

    //  if (typeDef != mdTypeRefNil) {
    //    logging::log(logging::LogLevel::INFO,
    //                 "Found app.Program"_W);

    //    HCORENUM hcorenum = 0;
    //    const auto maxMethods = 1000;
    //    mdMethodDef methods[maxMethods]{};
    //    ULONG cnt;
    //    metadataImport->EnumMethods(&hcorenum, typeDef, methods, maxMethods, &cnt);

    //    logging::log(logging::LogLevel::INFO,
    //                 "Found app.Program methods"_W);

    //    for (auto i = 0; i < cnt; i++) {
    //      const auto method_info = info::FunctionInfo::GetFunctionInfo(metadataImport, methods[i]);

    //      logging::log(logging::LogLevel::INFO,
    //                   "Found app.Program method {0}"_W, method_info.Name);

    //      if (method_info.Name == "TestM"_W) {
    //        logging::log(logging::LogLevel::INFO,
    //                     "Found app.Program method {0} request rejit"_W, method_info.Name);
    //        ModuleID m1[1]{moduleId};
    //        mdMethodDef m2[1]{methods[i]};
    //        hr = corProfilerInfo->RequestReJIT(1, m1, m2);
    //        if(FAILED(hr)) {
    //          logging::log(logging::LogLevel::INFO,
    //                       "Found app.Program method {0} request rejit failed"_W, method_info.Name);
    //        } else {
    //          logging::log(logging::LogLevel::INFO,
    //                       "Found app.Program method {0} request rejit success"_W, method_info.Name);
    //        }
    //      }
    //    }

    //  } else {
    //    logging::log(logging::LogLevel::INFO,
    //                 "Not Found app.Program"_W);
    //  }

    //  //         HCORENUM hcorenum;
    //  // const auto maxType = 256;
    //  // mdTypeRef typeRefs[maxType]{};
    //  // ULONG cnt;
    //  // metadataImport->EnumTypeRefs(&hcorenum, typeRefs, maxType, &cnt);

    //  // logging::log(logging::LogLevel::INFO,
    //  //              "Module {0} analyze got {1} types"_W, module_info.assembly.name, cnt);

    //  // mdToken parent = mdTokenNil;

    //  // for (auto i = 0; i < cnt; i++) {
    //  //   const auto maxNameSize = 1024;
    //  //   WCHAR name[maxNameSize]{};
    //  //   ULONG nameSize;
    //  //   metadataImport->GetTypeRefProps(typeRefs[i], &parent, name, maxNameSize, &nameSize);
    //  //   if (name == "app.Program"_W)
    //  //   {
    //  //     logging::log(logging::LogLevel::INFO,
    //  //                  "Class {0} found"_W, name);
    //  //   }
    //  // }
    //}

    return S_OK;
}
