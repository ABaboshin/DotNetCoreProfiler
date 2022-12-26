#include <algorithm>
#include "cor.h"
#include "corprof.h"
#include <corhlpr.h>
#include "MethodRewriter.h"
#include "logging/logging.h"
#include "CorProfiler.h"
#include "util/helpers.h"
#include "const/const.h"

HRESULT MethodRewriter::DefineLocalSignature(rewriter::ILRewriter *rewriter, ModuleID moduleId, mdTypeRef exceptionTypeRef, const RejitInfo &interceptor, ULONG *exceptionIndex, ULONG *returnIndex)
{
  HRESULT hr;

  ComPtr<IUnknown> metadataInterfaces;
  hr = profiler->corProfilerInfo->GetModuleMetaData(moduleId, ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf());
  if (FAILED(hr))
  {
    logging::log(logging::LogLevel::NONSUCCESS, "Failed GetModuleMetaData GetSigFromToken");
    return hr;
  }
  auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);
  auto metadataEmit = metadataInterfaces.As<IMetaDataEmit2>(IID_IMetaDataEmit);

  // modify local signature
  // to add an exception and the return value
  mdToken localVarSig = rewriter->GetTkLocalVarSig();
  PCCOR_SIGNATURE originalSignature = nullptr;
  ULONG originalSignatureSize = 0;

  if (localVarSig != mdTokenNil)
  {
    hr = metadataImport->GetSigFromToken(localVarSig, &originalSignature, &originalSignatureSize);
    if (FAILED(hr))
    {
      logging::log(logging::LogLevel::NONSUCCESS, "Failed DefineLocalSignature GetSigFromToken");
      return hr;
    }
  }

  // exception type buffer and size
  unsigned exceptionTypeRefBuffer;
  auto exceptionTypeRefSize = CorSigCompressToken(exceptionTypeRef, &exceptionTypeRefBuffer);

  // return type signature
  ULONG returnSignatureTypeSize = 0;
  mdTypeSpec returnValueTypeSpec = mdTypeSpecNil;

  auto newSignatureSize = originalSignatureSize + (1 + exceptionTypeRefSize);
  ULONG newSignatureOffset = 0;
  ULONG newLocalsCount = 1;

  auto returnSignature = interceptor.info.Signature.ReturnType.Raw;
  auto isVoid = interceptor.info.Signature.ReturnType.IsVoid;

  if (!isVoid)
  {
    returnSignatureTypeSize = returnSignature.size();

    mdTypeSpec returnValueTypeSpec = mdTypeSpecNil;
    hr = metadataEmit->GetTokenFromTypeSpec(&returnSignature[0], returnSignature.size(), &returnValueTypeSpec);
    newSignatureSize += (returnSignatureTypeSize);
    newLocalsCount++;
  }

  ULONG oldLocalsBuffer;
  ULONG oldLocalsLen = 0;
  unsigned newLocalsBuffer;
  ULONG newLocalsLen;

  if (originalSignatureSize == 0)
  {
    newSignatureSize += 2;
    newLocalsLen = CorSigCompressData(newLocalsCount, &newLocalsBuffer);
  }
  else
  {
    oldLocalsLen = CorSigUncompressData(originalSignature + 1, &oldLocalsBuffer);
    newLocalsCount += oldLocalsBuffer;
    newLocalsLen = CorSigCompressData(newLocalsCount, &newLocalsBuffer);
    newSignatureSize += newLocalsLen - oldLocalsLen;
  }

  // New signature declaration
  COR_SIGNATURE newSignatureBuffer[500];
  newSignatureBuffer[newSignatureOffset++] = IMAGE_CEE_CS_CALLCONV_LOCAL_SIG;

  // Set the locals count
  memcpy(&newSignatureBuffer[newSignatureOffset], &newLocalsBuffer, newLocalsLen);
  newSignatureOffset += newLocalsLen;

  if (originalSignatureSize > 0)
  {
    const auto copyLength = originalSignatureSize - 1 - oldLocalsLen;
    memcpy(&newSignatureBuffer[newSignatureOffset], originalSignature + 1 + oldLocalsLen, copyLength);
    newSignatureOffset += copyLength;
  }

  // exception
  newSignatureBuffer[newSignatureOffset++] = ELEMENT_TYPE_CLASS;
  memcpy(&newSignatureBuffer[newSignatureOffset], &exceptionTypeRefBuffer, exceptionTypeRefSize);
  newSignatureOffset += exceptionTypeRefSize;

  // return value if not void
  if (!isVoid)
  {
    memcpy(&newSignatureBuffer[newSignatureOffset], &returnSignature[0], returnSignatureTypeSize);
    newSignatureOffset += returnSignatureTypeSize;
  }

  // Get new locals token
  mdToken newLocalVarSig;
  hr = metadataEmit->GetTokenFromSig(newSignatureBuffer, newSignatureSize, &newLocalVarSig);
  if (FAILED(hr))
  {
    logging::log(logging::LogLevel::NONSUCCESS, "Failed local sig {0}"_W, interceptor.interceptor.Interceptor.AssemblyName);
    return hr;
  }

  rewriter->SetTkLocalVarSig(newLocalVarSig);

  *exceptionIndex = newLocalsCount - 1;
  *returnIndex = -1;
  if (!isVoid)
  {
    (*exceptionIndex)--;
    *returnIndex = newLocalsCount - 1;
  }

  return S_OK;
}

HRESULT MethodRewriter::InitLocalVariables(rewriter::ILRewriterHelper& helper, rewriter::ILRewriter* rewriter, util::ComPtr<IMetaDataEmit2>& metadataEmit, util::ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, ModuleID moduleId, const RejitInfo& interceptor, ULONG exceptionIndex, ULONG returnIndex) {
    HRESULT hr;

    // ex = null
    helper.LoadNull();
    helper.StLocal(exceptionIndex);

    auto isVoid = interceptor.info.Signature.ReturnType.IsVoid;

    if (!isVoid)
    {
        // define interceptor.dll
        mdModuleRef baseDllRef;
        hr = profiler->GetOrAddAssemblyRef(moduleId, profiler->configuration.DefaultInitializer.AssemblyName, baseDllRef);
        if (FAILED(hr))
        {
            logging::log(logging::LogLevel::NONSUCCESS, "Failed GetWrapperRef {0}"_W, profiler->configuration.DefaultInitializer.AssemblyName);
            return hr;
        }

        // define default initializer type
        mdTypeRef defaultInitializerTypeRef;
        hr = profiler->GetOrAddTypeRef(moduleId, baseDllRef, profiler->configuration.DefaultInitializer.TypeName.data(), defaultInitializerTypeRef);
        if (FAILED(hr))
        {
            logging::log(logging::LogLevel::NONSUCCESS, "Failed GetOrAddTypeRef {0}"_W, profiler->configuration.DefaultInitializer.TypeName);
            return hr;
        }

        std::vector<BYTE> memberSignature = {
            // see ECMA-355 II.23.2.15 MethodSpec
        IMAGE_CEE_CS_CALLCONV_GENERIC,
        // one generic argument
        1,
        // method without arguments
        0,
        ELEMENT_TYPE_MVAR,
        0 };

        // define generic GetDefault method
        mdMemberRef getDefaultRef;
        hr = metadataEmit->DefineMemberRef(
            defaultInitializerTypeRef,
            profiler->configuration.DefaultInitializer.MethodName.data(),
            memberSignature.data(),
            memberSignature.size(),
            &getDefaultRef);

        if (FAILED(hr))
        {
            logging::log(logging::LogLevel::NONSUCCESS, "Failed DefineMemberRef {0}"_W, profiler->configuration.DefaultInitializer.TypeName);
            return hr;
        }

        auto methodArgumentSignatureSize = interceptor.info.Signature.ReturnType.Raw.size();
        auto signatureLength = 2 + methodArgumentSignatureSize;

        COR_SIGNATURE signature[500];
        unsigned offset = 0;
        signature[offset++] = IMAGE_CEE_CS_CALLCONV_GENERICINST;
        signature[offset++] = 0x01;

        memcpy(&signature[offset], &interceptor.info.Signature.ReturnType.Raw[0], interceptor.info.Signature.ReturnType.Raw.size());
        offset += methodArgumentSignatureSize;

        mdMethodSpec getDefaultSpecRef = mdMethodSpecNil;
        hr = metadataEmit->DefineMethodSpec(getDefaultRef, signature, signatureLength, &getDefaultSpecRef);

        if (FAILED(hr))
        {
            logging::log(logging::LogLevel::NONSUCCESS, "Failed DefineMethodSpec {0}"_W, profiler->configuration.DefaultInitializer.TypeName);
            return hr;
        }

        helper.CallMember(getDefaultSpecRef, false);
        helper.StLocal(returnIndex);
    }

    return S_OK;
}
