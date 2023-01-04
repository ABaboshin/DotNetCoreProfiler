#include <algorithm>
#include "cor.h"
#include "corprof.h"
#include <corhlpr.h>
#include "MethodRewriter.h"
#include "logging/logging.h"
#include "CorProfiler.h"
#include "util/helpers.h"
#include "const/const.h"

struct greater
{
	template<class T>
	bool operator()(T const& a, T const& b) const { return a > b; }
};

HRESULT MethodRewriter::AddDebugger(rewriter::ILRewriterHelper& helper, util::ComPtr<IMetaDataEmit2>& metadataEmit, util::ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, const RejitInfo& interceptor)
{
	if (interceptor.Offsets.empty())
	{
		return S_OK;
	}

	auto offsets = interceptor.Offsets;

	// insert debugger from the end to start
	// to preserve the offsets
	std::sort(offsets.begin(), offsets.end(), greater());

	for (const auto el : offsets)
	{
		auto hr = AddDebugger(helper, metadataEmit, metadataAssemblyEmit, interceptor, el);
		if (FAILED(hr))
		{
			return hr;
		}
	}

	return S_OK;
}

HRESULT MethodRewriter::AddDebugger(rewriter::ILRewriterHelper& helper, util::ComPtr<IMetaDataEmit2>& metadataEmit, util::ComPtr<IMetaDataAssemblyEmit>& metadataAssemblyEmit, const RejitInfo& interceptor, int offset)
{
	logging::log(logging::LogLevel::INFO, "AddDebugger"_W);

	return S_OK;
}