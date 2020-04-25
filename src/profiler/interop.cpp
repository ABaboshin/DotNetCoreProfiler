#include "CorProfiler.h"

extern "C" void __cdecl AddInterceptor(ImportInterception interception)
{
	std::cout
		<< std::string(interception.CallerAssembly) << " "
		<< std::string(interception.TargetAssemblyName) << " "
		<< std::string(interception.TargetMethodName) << " "
		<< std::string(interception.TargetTypeName) << " "
		<< interception.TargetMethodParametersCount << " "
		<< std::string(interception.InterceptorTypeName) << " "
		<< std::string(interception.InterceptorMethodName) << " "
		<< std::string(interception.InterceptorAssemblyName) << " "
		<< interception.SignatureLength << " "
	<< std::endl;

	profiler->AddInterception(interception);
}