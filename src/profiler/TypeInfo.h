#pragma once

#include "cor.h"
#include "util.h"

struct TypeInfo {
  mdToken id;
  wstring name;

  TypeInfo() : id(0), name(""_W) {}
  TypeInfo(mdToken id, wstring name) : id(id), name(name) {}

  bool IsValid() const { return id != 0; }
};
