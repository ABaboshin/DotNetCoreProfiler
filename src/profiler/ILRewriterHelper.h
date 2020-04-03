#pragma once

#include "ILRewriter.h"

class ILRewriterHelper
{
private:
    ILRewriter* const m_ILRewriter;
    

public:

    ILInstr* m_ILInstr;

    ILRewriterHelper(ILRewriter* const il_rewriter)
        : m_ILRewriter(il_rewriter), m_ILInstr(nullptr) {}
    
    void SetILPosition(ILInstr* pILInstr) {
        m_ILInstr = pILInstr;
    }

    void SetLastILPosition(ILInstr* pILInstr) {
        auto pInstr = pILInstr;
        while (pInstr->m_pNext != nullptr)
        {
            pInstr = pInstr->m_pNext;
        }

        m_ILInstr = pInstr;
    }

    void LoadNull()
    {
        ILInstr* pNewInstr = m_ILRewriter->NewILInstr();
        pNewInstr->m_opcode = CEE_LDNULL;
        m_ILRewriter->InsertBefore(m_ILInstr, pNewInstr);
    }

    void StLocal(unsigned index)
    {
        static const std::vector<OPCODE> opcodes = {
               CEE_STLOC_0,
               CEE_STLOC_1,
               CEE_STLOC_2,
               CEE_STLOC_3,
        };

        ILInstr* pNewInstr = m_ILRewriter->NewILInstr();
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
        m_ILRewriter->InsertBefore(m_ILInstr, pNewInstr);
    }

    ILInstr* CreateArray(const mdTypeRef type_ref, const INT32 size)
    {
        LoadInt32(size);
        std::cout << "CEE_NEWARR" << std::endl;
        ILInstr* pNewInstr = m_ILRewriter->NewILInstr();
        pNewInstr->m_opcode = CEE_NEWARR;
        pNewInstr->m_Arg32 = type_ref;
        m_ILRewriter->InsertBefore(m_ILInstr, pNewInstr);
        return pNewInstr;
    }

    void Duplicate() const {
        std::cout << "CEE_DUP" << std::endl;
        ILInstr* pNewInstr = m_ILRewriter->NewILInstr();
        pNewInstr->m_opcode = CEE_DUP;
        m_ILRewriter->InsertBefore(m_ILInstr, pNewInstr);
    }

    void LoadInt32(const INT32 value) const {
        static const std::vector<OPCODE> opcodes = {
            CEE_LDC_I4_0, CEE_LDC_I4_1, CEE_LDC_I4_2, CEE_LDC_I4_3, CEE_LDC_I4_4,
            CEE_LDC_I4_5, CEE_LDC_I4_6, CEE_LDC_I4_7, CEE_LDC_I4_8,
        };

        ILInstr* pNewInstr = m_ILRewriter->NewILInstr();

        if (value >= 0 && value <= 8) {
            pNewInstr->m_opcode = opcodes[value];
            std::cout << "LoadInt32 " << opcodes[value] << std::endl;
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

        m_ILRewriter->InsertBefore(m_ILInstr, pNewInstr);
    }

    void BeginLoadValueIntoArray(const INT32 arrayIndex) const {
        
        // duplicate the array reference
        Duplicate();
        // load the specified array index
        LoadInt32(arrayIndex);
        
        
    }

    void LoadArgument(const UINT16 index) const {
        static const std::vector<OPCODE> opcodes = {
            CEE_LDARG_0,
            CEE_LDARG_1,
            CEE_LDARG_2,
            CEE_LDARG_3,
        };

        ILInstr* pNewInstr = m_ILRewriter->NewILInstr();

        if (index >= 0 && index <= 3) {
            pNewInstr->m_opcode = opcodes[index];
            std::cout << "LoadArgument " << opcodes[index] << std::endl;
        }
        else if (index <= 255) {
            pNewInstr->m_opcode = CEE_LDARG_S;
            pNewInstr->m_Arg8 = static_cast<UINT8>(index);
        }
        else {
            pNewInstr->m_opcode = CEE_LDARG;
            pNewInstr->m_Arg16 = index;
        }

        m_ILRewriter->InsertBefore(m_ILInstr, pNewInstr);
    }

    //https://github.com/dotnet/coreclr/blob/master/src/vm/stubgen.cpp EmitLDIND_T
    void LoadIND(unsigned elementType) const
    {
        unsigned op_code = 0;
        switch (elementType)
        {
        case ELEMENT_TYPE_I1:       op_code = CEE_LDIND_I1; break;
        case ELEMENT_TYPE_BOOLEAN:  // fall through
        case ELEMENT_TYPE_U1:       op_code = CEE_LDIND_U1; break;
        case ELEMENT_TYPE_I2:       op_code = CEE_LDIND_I2; break;
        case ELEMENT_TYPE_CHAR:     // fall through
        case ELEMENT_TYPE_U2:       op_code = CEE_LDIND_U2; break;
        case ELEMENT_TYPE_I4:       op_code = CEE_LDIND_I4; break;
        case ELEMENT_TYPE_U4:       op_code = CEE_LDIND_U4; break;
        case ELEMENT_TYPE_I8:       op_code = CEE_LDIND_I8; break;
        case ELEMENT_TYPE_U8:       op_code = CEE_LDIND_I8; break;
        case ELEMENT_TYPE_R4:       op_code = CEE_LDIND_R4; break;
        case ELEMENT_TYPE_R8:       op_code = CEE_LDIND_R8; break;
        case ELEMENT_TYPE_PTR:      // same as ELEMENT_TYPE_I
        case ELEMENT_TYPE_FNPTR:    // same as ELEMENT_TYPE_I
        case ELEMENT_TYPE_I:        op_code = CEE_LDIND_I;  break;
        case ELEMENT_TYPE_U:        op_code = CEE_LDIND_I;  break;
        case ELEMENT_TYPE_STRING:   // fall through
        case ELEMENT_TYPE_CLASS:    // fall through
        case ELEMENT_TYPE_ARRAY:
        case ELEMENT_TYPE_SZARRAY:
        case ELEMENT_TYPE_OBJECT:   op_code = CEE_LDIND_REF; break;

        case ELEMENT_TYPE_INTERNAL:
        {
            op_code = CEE_LDIND_REF;
            break;
        }
        default:
            break;
        }

        if (op_code > 0) {
            std::cout << "LoadIND " << op_code << std::endl;
            ILInstr* pNewInstr = m_ILRewriter->NewILInstr();
            pNewInstr->m_opcode = op_code;
            m_ILRewriter->InsertBefore(m_ILInstr, pNewInstr);
        }
    }

    void Box(const mdTypeRef type_ref) const {
        std::cout << CEE_BOX << std::endl;
        ILInstr* pNewInstr = m_ILRewriter->NewILInstr();
        pNewInstr->m_opcode = CEE_BOX;
        pNewInstr->m_Arg32 = type_ref;
        m_ILRewriter->InsertBefore(m_ILInstr, pNewInstr);
    }

    void EndLoadValueIntoArray() const {
        std::cout << "CEE_STELEM_REF" << std::endl;
        // stelem.ref (store value into array at the specified index)
        ILInstr* pNewInstr = m_ILRewriter->NewILInstr();
        pNewInstr->m_opcode = CEE_STELEM_REF;
        m_ILRewriter->InsertBefore(m_ILInstr, pNewInstr);
    }

    void CallMember(const mdMemberRef& member_ref,
        const bool is_virtual) const {
        ILInstr* pNewInstr = m_ILRewriter->NewILInstr();
        pNewInstr->m_opcode = is_virtual ? CEE_CALLVIRT : CEE_CALL;
        pNewInstr->m_Arg32 = member_ref;
        m_ILRewriter->InsertBefore(m_ILInstr, pNewInstr);
    }

    void Cast(const mdTypeRef type_ref) const {
        ILInstr* pNewInstr = m_ILRewriter->NewILInstr();
        pNewInstr->m_opcode = CEE_CASTCLASS;
        pNewInstr->m_Arg32 = type_ref;
        m_ILRewriter->InsertBefore(m_ILInstr, pNewInstr);
    }

    ILInstr* Rethrow() const
    {
        ILInstr* pNewInstr = m_ILRewriter->NewILInstr();
        pNewInstr->m_opcode = CEE_RETHROW;
        m_ILRewriter->InsertBefore(m_ILInstr, pNewInstr);
        return pNewInstr;
    }

    void LoadLocal(unsigned index) const
    {
        static const std::vector<OPCODE> opcodes = {
                CEE_LDLOC_0,
                CEE_LDLOC_1,
                CEE_LDLOC_2,
                CEE_LDLOC_3,
        };

        ILInstr* pNewInstr = m_ILRewriter->NewILInstr();
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
        m_ILRewriter->InsertBefore(m_ILInstr, pNewInstr);
    }

    ILInstr* EndFinally() const
    {
        ILInstr* pNewInstr = m_ILRewriter->NewILInstr();
        pNewInstr->m_opcode = CEE_ENDFINALLY;
        m_ILRewriter->InsertBefore(m_ILInstr, pNewInstr);
        return pNewInstr;
    }

    void UnboxAny(const mdTypeRef type_ref) const {
        ILInstr* pNewInstr = m_ILRewriter->NewILInstr();
        pNewInstr->m_opcode = CEE_UNBOX_ANY;
        pNewInstr->m_Arg32 = type_ref;
        m_ILRewriter->InsertBefore(m_ILInstr, pNewInstr);
    }

    ILInstr* CallMember0(const mdMemberRef& member_ref, bool is_virtual) const
    {
        ILInstr* pNewInstr = m_ILRewriter->NewILInstr();
        pNewInstr->m_opcode = is_virtual ? CEE_CALLVIRT : CEE_CALL;
        pNewInstr->m_Arg32 = member_ref;
        m_ILRewriter->InsertBefore(m_ILInstr, pNewInstr);
        return pNewInstr;
    }

    void LoadToken(mdToken token) const
    {
        ILInstr* pNewInstr = m_ILRewriter->NewILInstr();
        pNewInstr->m_opcode = CEE_LDTOKEN;
        pNewInstr->m_Arg32 = token;
        m_ILRewriter->InsertBefore(m_ILInstr, pNewInstr);
    }

    void NewObject(mdToken token) const
    {
        ILInstr* pNewInstr = m_ILRewriter->NewILInstr();
        pNewInstr->m_opcode = CEE_NEWOBJ;
        pNewInstr->m_Arg32 = token;
        m_ILRewriter->InsertBefore(m_ILInstr, pNewInstr);
    }

    void Nop()
    {
        std::cout << "CEE_NOP" << std::endl;
        ILInstr* pNewInstr = m_ILRewriter->NewILInstr();
        pNewInstr->m_opcode = CEE_NOP;
        m_ILRewriter->InsertBefore(m_ILInstr, pNewInstr);
    }

    void Pop()
    {
        ILInstr* pNewInstr = m_ILRewriter->NewILInstr();
        pNewInstr->m_opcode = CEE_POP;
        m_ILRewriter->InsertBefore(m_ILInstr, pNewInstr);
    }

    void Ret()
    {
        ILInstr* pNewInstr = m_ILRewriter->NewILInstr();
        pNewInstr->m_opcode = CEE_RET;
        m_ILRewriter->InsertBefore(m_ILInstr, pNewInstr);
    }
};