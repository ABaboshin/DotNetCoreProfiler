#pragma once

#include "ILRewriter.h"

namespace rewriter
{
    class ILRewriterHelper
    {
    private:
        ILRewriter* const _rewriter;
        ILInstr* _instr;

    public:
        ILRewriterHelper(ILRewriter* const rewriter)
            : _rewriter(rewriter), _instr(nullptr) {}

        void SetILPosition(ILInstr* instr);

        void StLocal(unsigned index);

        void CreateArray(const mdTypeRef typeRef, INT32 size);

        void Duplicate();

        void LoadInt32(INT32 value);

        void LoadInt64(INT64 value);

        void BeginLoadValueIntoArray(INT32 arrayIndex);

        void LoadArgument(UINT16 index);

        void EndLoadValueIntoArray();

        void CallMember(mdMemberRef memberRef, bool isVirtual);

        void Cast(mdTypeRef typeRef);

        void LoadLocal(unsigned index);

        void LoadLocalAddress(unsigned index);

        void NewObject(mdToken token);

        void Pop();

        void Ret();

        void LoadStr(mdToken token);
    };
}