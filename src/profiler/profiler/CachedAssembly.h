#pragma once

class CachedAssembly {
public:
	ModuleID ModuleId;
	util::wstring AssemblyName;
	mdModuleRef Ref;
	CachedAssembly(ModuleID moduleId, const util::wstring& assemblyName, mdModuleRef ref) : ModuleId(moduleId),
		AssemblyName(assemblyName),
		Ref(ref) {}
};
