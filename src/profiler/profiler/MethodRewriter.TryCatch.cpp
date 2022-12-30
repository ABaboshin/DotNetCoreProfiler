#include <algorithm>
#include "cor.h"
#include "corprof.h"
#include <corhlpr.h>
#include "MethodRewriter.h"
#include "logging/logging.h"
#include "CorProfiler.h"
#include "util/helpers.h"
#include "const/const.h"

HRESULT MethodRewriter::CreateTryCatch(TBlock tryBlock, TBlock catchBlock, TLeaveInstructionModifier tryModifier, TLeaveInstructionModifier catchModifier, mdTypeRef exceptionTypeRef, rewriter::EHClause& ehClause)
{
	rewriter::ILInstr* tryBegin = nullptr;
	rewriter::ILInstr* tryLeave = nullptr;
	rewriter::ILInstr* catchBegin = nullptr;
	rewriter::ILInstr* catchLeave = nullptr;

	tryBlock(&tryBegin, &tryLeave);
	catchBlock(&catchBegin, &catchLeave);

	std::cout << std::hex << tryBegin << " " << tryLeave << " " << catchBegin << " " << catchLeave << std::endl;

	tryModifier(*tryLeave);
	catchModifier(*catchLeave);


	ehClause.m_Flags = COR_ILEXCEPTION_CLAUSE_NONE;
	ehClause.m_pTryBegin = tryBegin;
	ehClause.m_pTryEnd = catchBegin;
	ehClause.m_pHandlerBegin = catchBegin;
	ehClause.m_pHandlerEnd = catchLeave;
	ehClause.m_ClassToken = exceptionTypeRef;

	return S_OK;
}