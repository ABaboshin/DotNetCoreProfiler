#pragma once

class CachedType {
public:
	ModuleID ModuleId;
	mdModuleRef ModuleRef;
  util::wstring TypeName;
  mdTypeRef TypeRef;
  CachedType(ModuleID moduleId, mdModuleRef moduleRef, const util::wstring &typeName, mdTypeRef typeRef) : ModuleId(moduleId),
                                                                                                           TypeName(typeName),
                                                                                                           ModuleRef(moduleRef), TypeRef(typeRef) {}
};
