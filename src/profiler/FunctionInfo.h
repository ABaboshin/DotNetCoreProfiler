#pragma once

#include "MethodSignature.h"
#include "TypeInfo.h"

struct FunctionInfo {
  const mdToken id;
  const WSTRING name;
  const TypeInfo type;
  const BOOL is_generic;
  const MethodSignature signature;
  const MethodSignature function_spec_signature;
  const mdToken method_def_id;

  FunctionInfo()
      : id(0), name(""_W), type({}), is_generic(false), method_def_id(0) {}

  FunctionInfo(mdToken id, WSTRING name, TypeInfo type,
               MethodSignature signature,
               MethodSignature function_spec_signature, mdToken method_def_id)
      : id(id),
        name(name),
        type(type),
        is_generic(true),
        signature(signature),
        function_spec_signature(function_spec_signature),
        method_def_id(method_def_id) {}

  FunctionInfo(mdToken id, WSTRING name, TypeInfo type,
               MethodSignature signature)
      : id(id),
        name(name),
        type(type),
        is_generic(false),
        signature(signature),
        method_def_id(0) {}

  bool IsValid() const { return id != 0; }
};
