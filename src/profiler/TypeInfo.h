#pragma once

#include "util.h"

struct TypeInfo {
  const mdToken id;
  const WSTRING name;

  TypeInfo() : id(0), name(""_W) {}
  TypeInfo(mdToken id, WSTRING name) : id(id), name(name) {}

  bool IsValid() const { return id != 0; }
};
