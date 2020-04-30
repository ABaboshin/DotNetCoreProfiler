#include "ILRewriterHelper.h"
#include <vector>

void ILRewriterHelper::SetILPosition(ILInstr* instr)
{
    _instr = instr;
}

void ILRewriterHelper::StLocal(unsigned index)
{
    static const std::vector<OPCODE> opcodes = {
           CEE_STLOC_0,
           CEE_STLOC_1,
           CEE_STLOC_2,
           CEE_STLOC_3,
    };

    ILInstr* pNewInstr = _rewriter->NewILInstr();
    if (index <= 3) {
        pNewInstr->m_opcode = opcodes[index];
    }
    else if (index <= 255) {
        pNewInstr->m_opcode = CEE_STLOC_S;
        pNewInstr->m_Arg8 = static_cast<UINT8>(index);
    }
    else {
        pNewInstr->m_opcode = CEE_STLOC;
        pNewInstr->m_Arg16 = index;
    }
    _rewriter->InsertBefore(_instr, pNewInstr);
}

void ILRewriterHelper::CreateArray(const mdTypeRef typeRef, INT32 size)
{
    if (size > 0) LoadInt32(size);
    ILInstr* pNewInstr = _rewriter->NewILInstr();
    pNewInstr->m_opcode = CEE_NEWARR;
    pNewInstr->m_Arg32 = typeRef;
    _rewriter->InsertBefore(_instr, pNewInstr);
}

void ILRewriterHelper::Duplicate()
{
    ILInstr* pNewInstr = _rewriter->NewILInstr();
    pNewInstr->m_opcode = CEE_DUP;
    _rewriter->InsertBefore(_instr, pNewInstr);
}

void ILRewriterHelper::LoadInt32(INT32 value)
{
    static const std::vector<OPCODE> opcodes = {
        CEE_LDC_I4_0, CEE_LDC_I4_1, CEE_LDC_I4_2, CEE_LDC_I4_3, CEE_LDC_I4_4,
        CEE_LDC_I4_5, CEE_LDC_I4_6, CEE_LDC_I4_7, CEE_LDC_I4_8,
    };

    ILInstr* pNewInstr = _rewriter->NewILInstr();

    if (value >= 0 && value <= 8) {
        pNewInstr->m_opcode = opcodes[value];
    }
    else if (value == -1) {
        pNewInstr->m_opcode = CEE_LDC_I4_M1;
    }
    else if (-128 <= value && value <= 127) {
        pNewInstr->m_opcode = CEE_LDC_I4_S;
        pNewInstr->m_Arg8 = static_cast<INT8>(value);
    }
    else {
        pNewInstr->m_opcode = CEE_LDC_I4;
        pNewInstr->m_Arg32 = value;
    }

    _rewriter->InsertBefore(_instr, pNewInstr);
}

void ILRewriterHelper::LoadInt64(INT64 value)
{
    ILInstr* pNewInstr = _rewriter->NewILInstr();
    pNewInstr->m_opcode = CEE_LDC_I8;
    pNewInstr->m_Arg64 = value;
    _rewriter->InsertBefore(_instr, pNewInstr);
}

void ILRewriterHelper::BeginLoadValueIntoArray(INT32 arrayIndex)
{
    Duplicate();
    LoadInt32(arrayIndex);
}

void ILRewriterHelper::LoadArgument(UINT16 index)
{
    static const std::vector<OPCODE> opcodes = {
        CEE_LDARG_0,
        CEE_LDARG_1,
        CEE_LDARG_2,
        CEE_LDARG_3,
    };

    ILInstr* pNewInstr = _rewriter->NewILInstr();

    if (index >= 0 && index <= 3) {
        pNewInstr->m_opcode = opcodes[index];
    }
    else if (index <= 255) {
        pNewInstr->m_opcode = CEE_LDARG_S;
        pNewInstr->m_Arg8 = static_cast<UINT8>(index);
    }
    else {
        pNewInstr->m_opcode = CEE_LDARG;
        pNewInstr->m_Arg16 = index;
    }

    _rewriter->InsertBefore(_instr, pNewInstr);
}

void ILRewriterHelper::EndLoadValueIntoArray()
{
    // stelem.ref (store value into array at the specified index)
    ILInstr* pNewInstr = _rewriter->NewILInstr();
    pNewInstr->m_opcode = CEE_STELEM_REF;
    _rewriter->InsertBefore(_instr, pNewInstr);
}

void ILRewriterHelper::CallMember(mdMemberRef memberRef, bool isVirtual)
{
    ILInstr* pNewInstr = _rewriter->NewILInstr();
    pNewInstr->m_opcode = isVirtual ? CEE_CALLVIRT : CEE_CALL;
    pNewInstr->m_Arg32 = memberRef;
    _rewriter->InsertBefore(_instr, pNewInstr);
}

void ILRewriterHelper::Cast(mdTypeRef typeRef)
{
    ILInstr* pNewInstr = _rewriter->NewILInstr();
    pNewInstr->m_opcode = CEE_CASTCLASS;
    pNewInstr->m_Arg32 = typeRef;
    _rewriter->InsertBefore(_instr, pNewInstr);
}

void ILRewriterHelper::LoadLocal(unsigned index)
{
    static const std::vector<OPCODE> opcodes = {
            CEE_LDLOC_0,
            CEE_LDLOC_1,
            CEE_LDLOC_2,
            CEE_LDLOC_3,
    };

    ILInstr* pNewInstr = _rewriter->NewILInstr();
    if (index <= 3) {
        pNewInstr->m_opcode = opcodes[index];
    }
    else if (index <= 255) {
        pNewInstr->m_opcode = CEE_LDLOC_S;
        pNewInstr->m_Arg8 = static_cast<UINT8>(index);
    }
    else {
        pNewInstr->m_opcode = CEE_LDLOC;
        pNewInstr->m_Arg16 = index;
    }
    _rewriter->InsertBefore(_instr, pNewInstr);
}

void ILRewriterHelper::LoadLocalAddress(unsigned index)
{
    ILInstr* pNewInstr = _rewriter->NewILInstr();
    if (index <= 255) {
        pNewInstr->m_opcode = CEE_LDLOCA_S;
        pNewInstr->m_Arg8 = static_cast<UINT8>(index);
    }
    else {
        pNewInstr->m_opcode = CEE_LDLOCA;
        pNewInstr->m_Arg16 = index;
    }
    _rewriter->InsertBefore(_instr, pNewInstr);
}

void ILRewriterHelper::NewObject(mdToken token)
{
    ILInstr* pNewInstr = _rewriter->NewILInstr();
    pNewInstr->m_opcode = CEE_NEWOBJ;
    pNewInstr->m_Arg32 = token;
    _rewriter->InsertBefore(_instr, pNewInstr);
}

void ILRewriterHelper::Pop()
{
    ILInstr* pNewInstr = _rewriter->NewILInstr();
    pNewInstr->m_opcode = CEE_POP;
    _rewriter->InsertBefore(_instr, pNewInstr);
}

void ILRewriterHelper::Ret()
{
    ILInstr* pNewInstr = _rewriter->NewILInstr();
    pNewInstr->m_opcode = CEE_RET;
    _rewriter->InsertBefore(_instr, pNewInstr);
}

void ILRewriterHelper::LoadStr(mdToken token)
{
    ILInstr* pNewInstr = _rewriter->NewILInstr();
    pNewInstr->m_opcode = CEE_LDSTR;
    pNewInstr->m_Arg32 = token;
    _rewriter->InsertBefore(_instr, pNewInstr);
}