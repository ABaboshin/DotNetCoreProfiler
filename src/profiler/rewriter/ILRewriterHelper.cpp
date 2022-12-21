#include <vector>
#include "ILRewriterHelper.h"
#include "util/ComPtr.h"
#include "util/util.h"
#include "info/parser.h"

namespace rewriter
{
    void ILRewriterHelper::SetILPosition(ILInstr* instr)
    {
        _instr = instr;
    }

    ILInstr* ILRewriterHelper::StLocal(unsigned index)
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
        return pNewInstr;
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

    ILInstr* ILRewriterHelper::LoadInt32(INT32 value)
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
        return pNewInstr;
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

    void ILRewriterHelper::LoadArgumentRef(UINT16 index)
    {
        ILInstr* pNewInstr = _rewriter->NewILInstr();

        if (index <= 255) {
            pNewInstr->m_opcode = CEE_LDARGA_S;
            pNewInstr->m_Arg8 = static_cast<UINT8>(index);
        }
        else {
            pNewInstr->m_opcode = CEE_LDARGA;
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

    ILInstr* ILRewriterHelper::LoadLocal(unsigned index)
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
        return pNewInstr;
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

    void ILRewriterHelper::LoadObj(mdToken token) {
        ILInstr* pNewInstr = _rewriter->NewILInstr();
        pNewInstr->m_opcode = CEE_LDOBJ;
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

	void ILRewriterHelper::Box(mdToken token)
	{
        ILInstr* pNewInstr = _rewriter->NewILInstr();
        pNewInstr->m_opcode = CEE_BOX;
        pNewInstr->m_Arg32 = token;
        _rewriter->InsertBefore(_instr, pNewInstr);
	}

    void ILRewriterHelper::LoadInd(BYTE typeDef)
    {
        unsigned opCode = 0;
        switch (typeDef)
        {
        case ELEMENT_TYPE_I1:
            opCode = CEE_LDIND_I1;
            break;
        case ELEMENT_TYPE_BOOLEAN:
        case ELEMENT_TYPE_U1:
            opCode = CEE_LDIND_U1;
            break;
        case ELEMENT_TYPE_I2:
            opCode = CEE_LDIND_I2;
            break;
        case ELEMENT_TYPE_CHAR:
        case ELEMENT_TYPE_U2:
            opCode = CEE_LDIND_U2;
            break;
        case ELEMENT_TYPE_I4:
            opCode = CEE_LDIND_I4;
            break;
        case ELEMENT_TYPE_U4:
            opCode = CEE_LDIND_U4;
            break;
        case ELEMENT_TYPE_I8:
            opCode = CEE_LDIND_I8;
            break;
        case ELEMENT_TYPE_U8:
            opCode = CEE_LDIND_I8;
            break;
        case ELEMENT_TYPE_R4:
            opCode = CEE_LDIND_R4;
            break;
        case ELEMENT_TYPE_R8:
            opCode = CEE_LDIND_R8;
            break;
        case ELEMENT_TYPE_PTR:
        case ELEMENT_TYPE_FNPTR:
        case ELEMENT_TYPE_I:
        case ELEMENT_TYPE_U:
            opCode = CEE_LDIND_I;
            break;
        case ELEMENT_TYPE_STRING:
        case ELEMENT_TYPE_CLASS:
        case ELEMENT_TYPE_ARRAY:
        case ELEMENT_TYPE_SZARRAY:
        case ELEMENT_TYPE_OBJECT:
        case ELEMENT_TYPE_INTERNAL:
            opCode = CEE_LDIND_REF;
            break;
        default:
            break;
        }

        if (opCode > 0) {
            ILInstr* pNewInstr = _rewriter->NewILInstr();
            pNewInstr->m_opcode = opCode;
            _rewriter->InsertBefore(_instr, pNewInstr);
        }
    }

    void ILRewriterHelper::StInd(BYTE typeDef)
    {
        unsigned opCode = 0;
        switch (typeDef)
        {
        case ELEMENT_TYPE_I1:
        case ELEMENT_TYPE_BOOLEAN:
        case ELEMENT_TYPE_U1:
            opCode = CEE_STIND_I1;
            break;
        
        case ELEMENT_TYPE_I2:
        case ELEMENT_TYPE_CHAR:
        case ELEMENT_TYPE_U2:
            opCode = CEE_STIND_I2;
            break;
        case ELEMENT_TYPE_I4:
        case ELEMENT_TYPE_U4:
            opCode = CEE_STIND_I4;
            break;
        case ELEMENT_TYPE_I8:
        case ELEMENT_TYPE_U8:
            opCode = CEE_STIND_I8;
            break;
        case ELEMENT_TYPE_R4:
            opCode = CEE_STIND_R4;
            break;
        case ELEMENT_TYPE_R8:
            opCode = CEE_STIND_R8;
            break;
        case ELEMENT_TYPE_PTR:
        case ELEMENT_TYPE_FNPTR:
        case ELEMENT_TYPE_I:
        case ELEMENT_TYPE_U:
            opCode = CEE_STIND_I;
            break;
        case ELEMENT_TYPE_STRING:
        case ELEMENT_TYPE_CLASS:
        case ELEMENT_TYPE_ARRAY:
        case ELEMENT_TYPE_SZARRAY:
        case ELEMENT_TYPE_OBJECT:
        case ELEMENT_TYPE_INTERNAL:
            opCode = CEE_STIND_REF;
            break;
        default:
            break;
        }

        if (opCode > 0) {
            ILInstr* pNewInstr = _rewriter->NewILInstr();
            pNewInstr->m_opcode = opCode;
            _rewriter->InsertBefore(_instr, pNewInstr);
        }
    }

    ILInstr* ILRewriterHelper::BrTrueS()
    {
        ILInstr* pNewInstr = _rewriter->NewILInstr();
        pNewInstr->m_opcode = CEE_BRTRUE_S;
        _rewriter->InsertBefore(_instr, pNewInstr);
        return pNewInstr;
    }

    ILInstr* ILRewriterHelper::BrFalseS()
    {
        ILInstr* pNewInstr = _rewriter->NewILInstr();
        pNewInstr->m_opcode = CEE_BRFALSE_S;
        _rewriter->InsertBefore(_instr, pNewInstr);
        return pNewInstr;
    }

    ILInstr* ILRewriterHelper::BrS()
    {
        ILInstr* pNewInstr = _rewriter->NewILInstr();
        pNewInstr->m_opcode = CEE_BR_S;
        _rewriter->InsertBefore(_instr, pNewInstr);
        return pNewInstr;
    }

    ILInstr* ILRewriterHelper::Nop()
    {
        ILInstr* pNewInstr = _rewriter->NewILInstr();
        pNewInstr->m_opcode = CEE_NOP;
        _rewriter->InsertBefore(_instr, pNewInstr);
        return pNewInstr;
    }

    ILInstr* ILRewriterHelper::LoadNull()
    {
        ILInstr* pNewInstr = _rewriter->NewILInstr();
        pNewInstr->m_opcode = CEE_LDNULL;
        _rewriter->InsertBefore(_instr, pNewInstr);
        return pNewInstr;
    }

    ILInstr* ILRewriterHelper::CgtUn()
    {
        ILInstr* pNewInstr = _rewriter->NewILInstr();
        pNewInstr->m_opcode = CEE_CGT_UN;
        _rewriter->InsertBefore(_instr, pNewInstr);
        return pNewInstr;
    }

    ILInstr* ILRewriterHelper::Throw()
    {
        ILInstr* pNewInstr = _rewriter->NewILInstr();
        pNewInstr->m_opcode = CEE_THROW;
        _rewriter->InsertBefore(_instr, pNewInstr);
        return pNewInstr;
    }

    HRESULT ILRewriterHelper::AddLocalVariable(mdTypeRef typeRef, int& newIndex)
    {
        HRESULT hr;

        util::ComPtr<IUnknown> metadataInterfaces;
        IfFailRet(_rewriter->GetCorProfilerInfo()->GetModuleMetaData(_rewriter->GetModuleId(), ofRead | ofWrite, IID_IMetaDataImport, metadataInterfaces.GetAddressOf()));

        auto metadataImport = metadataInterfaces.As<IMetaDataImport2>(IID_IMetaDataImport);
        auto metadataEmit = metadataInterfaces.As<IMetaDataEmit2>(IID_IMetaDataEmit);

        ULONG temp = 0;

        std::vector<BYTE> signature{};
        signature.push_back(IMAGE_CEE_CS_CALLCONV_LOCAL_SIG);

        // read existing local sig if any
        if (_rewriter->GetTkLocalVarSig() != mdTokenNil)
        {
            std::vector<BYTE> originalSignature{};
            PCCOR_SIGNATURE originalSignatureRaw = nullptr;
            ULONG originalSignatureSize = 0;
            IfFailRet(metadataImport->GetSigFromToken(_rewriter->GetTkLocalVarSig(), &originalSignatureRaw, &originalSignatureSize));

            originalSignature = util::ToRaw(originalSignatureRaw, originalSignatureSize);
            auto iter = originalSignature.begin();
            std::advance(iter, 1);

            // increment local var count
            ULONG originalVariablesCount = 0;
            info::ParseNumber(iter, originalVariablesCount);
            originalVariablesCount += 1;

            newIndex = originalVariablesCount - 1;

            BYTE compressedToken[10];
            auto newLength = CorSigCompressData(originalVariablesCount, &compressedToken);

            // add it to declaration
            signature.insert(signature.end(), compressedToken, compressedToken + newLength);

            // insert existing variables
            signature.insert(signature.end(), iter, originalSignature.end());
        }
        else
        {
            BYTE compressedToken[10];
            auto newLength = CorSigCompressData(1, &compressedToken);

            signature.insert(signature.end(), compressedToken, compressedToken + newLength);

            newIndex = 0;
        }

        BYTE compressedTypeToken[10];
        ULONG compressedTypeTokenLength = CorSigCompressToken(typeRef, compressedTypeToken);

        signature.push_back(ELEMENT_TYPE_CLASS);
        signature.insert(signature.end(), compressedTypeToken, compressedTypeToken + compressedTypeTokenLength);

        mdToken newSigToken;

        hr = metadataEmit->GetTokenFromSig(signature.data(), signature.size(), &newSigToken);

        _rewriter->SetTkLocalVarSig(newSigToken);

        return S_OK;
    }

    void ILRewriterHelper::UnboxAny(mdTypeRef typeRef)
    {
        ILInstr* pNewInstr = _rewriter->NewILInstr();
        pNewInstr->m_opcode = CEE_UNBOX_ANY;
        pNewInstr->m_Arg32 = typeRef;
    }
}