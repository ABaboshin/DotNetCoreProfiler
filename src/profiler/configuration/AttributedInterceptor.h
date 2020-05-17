#pragma once
#include "configuration/Interceptor.h"

namespace configuration
{
	struct AttributedInterceptor
	{
		Interceptor Interceptor{};
		wstring AttributeType;
		bool ParameterLevel;

		AttributedInterceptor(const ::configuration::Interceptor& interceptor, const wstring& attributeType, bool parameterLevel) :
			Interceptor(interceptor),
			AttributeType(attributeType),
			ParameterLevel(parameterLevel) {}

		AttributedInterceptor() {}
	};
}