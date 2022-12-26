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

        ILInstr* StLocal(unsigned index);

        void CreateArray(const mdTypeRef typeRef, INT32 size);

        void Duplicate();

        ILInstr* LoadInt32(INT32 value);

        void LoadInt64(INT64 value);

        void BeginLoadValueIntoArray(INT32 arrayIndex);

        ILInstr* LoadArgument(UINT16 index);
        void LoadArgumentRef(UINT16 index);

        void EndLoadValueIntoArray();

        ILInstr* CallMember(mdMemberRef memberRef, bool isVirtual);

        void Cast(mdTypeRef typeRef);

        ILInstr* LoadLocal(unsigned index);

        void LoadLocalAddress(unsigned index);

        void NewObject(mdToken token);

        ILInstr* Pop();

        void Ret();

        void LoadStr(mdToken token);

        void Box(mdToken token);

        void UnboxAny(mdTypeRef typeRef);

        void LoadInd(BYTE typeDef);

        void StInd(BYTE typeDef);

        ILInstr* BrTrueS();

        ILInstr* BrFalseS();

        ILInstr* BrS();

        ILInstr* Nop();

        ILInstr* LoadNull();

        ILInstr* Throw();
        ILInstr* Rethrow();
        ILInstr* EndFinally();

        HRESULT AddLocalVariable(mdTypeRef typeRef, int& newIndex);

        void LoadObj(mdToken token);

        ILInstr* LeaveS();

        ILInstr* GetCurrentInstr() const
        {
            return _instr;
        }

        ILInstr* NewInstr() {
            _instr++;
            return new ILInstr();
        }
    };
}