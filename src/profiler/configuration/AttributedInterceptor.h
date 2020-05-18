#pragma once
#include "configuration/Interceptor.h"

namespace configuration
{
	struct AttributedInterceptor
	{
		Interceptor Interceptor{};
		wstring AttributeType;

		AttributedInterceptor(const ::configuration::Interceptor& interceptor, const wstring& attributeType) :
			Interceptor(interceptor),
			AttributeType(attributeType) {}

		AttributedInterceptor() {}
	};
}