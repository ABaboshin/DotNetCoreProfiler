#pragma once
#include "configuration/TypeInfo.h"

namespace configuration
{
	struct AttributedInterceptor
	{
		TypeInfo Interceptor{};
		wstring AttributeType;

		AttributedInterceptor(const ::configuration::TypeInfo& interceptor, const wstring& attributeType) :
			Interceptor(interceptor),
			AttributeType(attributeType) {}

		AttributedInterceptor() {}
	};
}