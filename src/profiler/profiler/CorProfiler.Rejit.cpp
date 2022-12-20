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

HRESULT STDMETHODCALLTYPE CorProfiler::ReJITCompilationFinished(FunctionID functionId, ReJITID rejitId, HRESULT hrStatus, BOOL fIsSafeToBlock)
{
    logging::log(
        logging::LogLevel::INFO,
        "ReJITCompilationFinished"_W);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ReJITError(ModuleID moduleId, mdMethodDef methodId, FunctionID functionId, HRESULT hrStatus)
{
    logging::log(
        logging::LogLevel::INFO,
        "ReJITError"_W);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::ReJITCompilationStarted(FunctionID functionId, ReJITID rejitId, BOOL fIsSafeToBlock)
{
    logging::log(
        logging::LogLevel::INFO,
        "ReJITCompilationStarted"_W);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfiler::GetReJITParameters(ModuleID moduleId, mdMethodDef methodId, ICorProfilerFunctionControl* pFunctionControl)
{
    std::lock_guard<std::mutex> guard(mutex);

    HRESULT hr;
    mdToken functionToken;
    ClassID classId;
    // ModuleID moduleId;

    const auto interceptor = std::find_if(rejitInfo.begin(), rejitInfo.end(), [moduleId, methodId](const RejitInfo& ri) {
        return ri.methodId == methodId && ri.moduleId == moduleId;
    });

    // no rejit info => exit
    if (interceptor == rejitInfo.end()) return S_OK;

    logging::log(
        logging::LogLevel::INFO,
        "GetReJITParameters {0} {1} {2} {3}"_W, moduleId, methodId, pFunctionControl != NULL, this->corProfilerInfo != NULL);

    auto rewriter = CreateILRewriter(pFunctionControl, moduleId, methodId);
    hr = rewriter->Import();
    if (FAILED(hr))
    {
        logging::log(
            logging::LogLevel::INFO,
            "GetReJITParameters rewriter->Import failed"_W);
        return S_OK;
    }

    // auto rewriter = CreateILRewriter(NULL, moduleId, methodId);
    // rewriter->InitializeTiny();
    //rewriter::ILRewriterHelper helper(rewriter);
    //// helper.Ret();

    //for (rewriter::ILInstr *pInstr = rewriter->GetILList()->m_pNext;
    //     pInstr != rewriter->GetILList(); pInstr = pInstr->m_pNext)
    //{
    //  if (pInstr->m_opcode != rewriter::CEE_CALL && pInstr->m_opcode != rewriter::CEE_CALLVIRT && pInstr->m_opcode != rewriter::CEE_RET)
    //  {
    //    continue;
    //  }

    //  pInstr->m_opcode = rewriter::CEE_NOP;
    //}

    //hr = rewriter->Export();

    //if (FAILED(hr))
    //{
    //  logging::log(
    //      logging::LogLevel::INFO,
    //      "GetReJITParameters rewriter->Export failed"_W);
    //  return S_OK;
    //}

    return S_OK;

    // for (rewriter::ILInstr *pInstr = rewriter->GetILList()->m_pNext;
    //      pInstr != rewriter->GetILList(); pInstr = pInstr->m_pNext)
    // {
    //   std::cout << std::hex << pInstr->m_opcode << std::endl;
    // }

    //TODO save method metadata when requesting rejitting
    // hr = this->corProfilerInfo->GetFunctionInfo(methodId, &classId, &moduleId, &functionToken);
    // if (FAILED(hr)) {
    //   logging::log(
    //       logging::LogLevel::INFO,
    //       "GetReJITParameters GetFunctionInfo failed"_W);
    //   return S_OK;
    // }

    // auto moduleInfo = info::ModuleInfo::GetModuleInfo(this->corProfilerInfo, moduleId);

    // logging::log(
    //     logging::LogLevel::INFO,
    //     "GetReJITParameters GetModuleInfo done"_W);
    // ComPtr<IUnknown> metadataInterfaces;
    // hr = this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf());
    // if (FAILED(hr)) {
    //   logging::log(
    //       logging::LogLevel::INFO,
    //       "GetReJITParameters GetModuleMetaData failed"_W);
    //   return S_OK;
    // }

    // const auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);

    // const auto functionInfo = info::FunctionInfo::GetFunctionInfo(metadataImport, functionToken);

    // logging::log(
    //     logging::LogLevel::INFO,
    //     "GetReJITParameters for app_domain_id {0} method {1}.{2} num args {3} from assembly {4}"_W,
    //     moduleInfo.assembly.appDomainId,
    //     functionInfo.Type.Name,
    //     functionInfo.Name,
    //     functionInfo.Signature.NumberOfArguments(),
    //     moduleInfo.assembly.name);

    return S_OK;
}
