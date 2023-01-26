#include <sstream>
#include <functional>
#include "CorProfiler.h"

using namespace util;

util::wstring CorProfiler::DumpILCodes(rewriter::ILRewriter* rewriter, const info::FunctionInfo& functionInfo, const ComPtr<IMetaDataImport2>& metadataImport)
{
	try
	{
        std::stringstream stream;
        stream << util::ToString(functionInfo.Type.Name) << "." << util::ToString(functionInfo.Name) << std::endl;

        const auto ehCount = rewriter->GetEHCount();
        const auto ehPointer = rewriter->GetEHPointer();
        int indent = 1;

        std::function<std::string(int)> createIndent = [](int i) { return i > 0 ? std::string(i, ' ') : ""; };

        PCCOR_SIGNATURE originalSignature = nullptr;
        ULONG originalSignatureSize = 0;
        mdToken localVarSig = rewriter->GetTkLocalVarSig();

        if (localVarSig != mdTokenNil)
        {
            auto hr = metadataImport->GetSigFromToken(localVarSig, &originalSignature, &originalSignatureSize);
            if (SUCCEEDED(hr))
            {
                stream << "Local Var Signature: " << util::HexStr(originalSignature, originalSignatureSize) << std::endl;
            }
        }

        for (auto cInstr = rewriter->GetILList()->m_pNext; cInstr != rewriter->GetILList(); cInstr = cInstr->m_pNext)
        {

            if (ehCount > 0)
            {
                for (auto i = 0; i < ehCount; i++)
                {
                    if (ehPointer[i].m_Flags == COR_ILEXCEPTION_CLAUSE_FINALLY)
                    {
                        if (ehPointer[i].m_pTryBegin == cInstr)
                        {
                            stream << createIndent(indent) << ".try {" << std::endl;
                            indent++;
                        }
                        if (ehPointer[i].m_pTryEnd == cInstr)
                        {
                            indent--;
                            stream << createIndent(indent) << "}" << std::endl;
                        }
                        if (ehPointer[i].m_pHandlerBegin == cInstr)
                        {
                            stream << createIndent(indent) << ".finally {" << std::endl;
                            indent++;
                        }
                    }
                }
                for (auto i = 0; i < ehCount; i++)
                {
                    const auto& currentEH = ehPointer[i];
                    if (ehPointer[i].m_Flags == COR_ILEXCEPTION_CLAUSE_NONE)
                    {
                        if (ehPointer[i].m_pTryBegin == cInstr)
                        {
                            stream << createIndent(indent) << ".try {" << std::endl;
                            indent++;
                        }
                        if (ehPointer[i].m_pTryEnd == cInstr)
                        {
                            indent--;
                            stream << createIndent(indent) << "}" << std::endl;
                        }
                        if (ehPointer[i].m_pHandlerBegin == cInstr)
                        {
                            stream << createIndent(indent) << ".catch {" << std::endl;
                            indent++;
                        }
                    }
                }
            }

            stream << createIndent(indent) << cInstr << ": ";
            if (cInstr->m_opcode < opCodes.size())
            {
                stream << std::setw(10) << opCodes[cInstr->m_opcode];
            }
            else
            {
                stream << "0x";
                stream << std::setfill('0') << std::setw(2) << std::hex << cInstr->m_opcode;
            }
            if (cInstr->m_pTarget != NULL)
            {
                stream << "  " << cInstr->m_pTarget;

                if (cInstr->m_opcode == rewriter::OPCODE::CEE_CALL || cInstr->m_opcode == rewriter::OPCODE::CEE_CALLVIRT || cInstr->m_opcode == rewriter::OPCODE::CEE_NEWOBJ)
                {
                    const auto memberInfo = info::FunctionInfo::GetFunctionInfo(metadataImport, (mdMemberRef)cInstr->m_Arg32);
                    stream << "  | " << util::ToString(memberInfo.Type.Name) << "." << util::ToString(memberInfo.Name) << "(" << memberInfo.Signature.NumberOfArguments() << " argument)";
                }
                else if (cInstr->m_opcode == rewriter::OPCODE::CEE_CASTCLASS || cInstr->m_opcode == rewriter::OPCODE::CEE_BOX ||
                    cInstr->m_opcode == rewriter::OPCODE::CEE_UNBOX_ANY || cInstr->m_opcode == rewriter::OPCODE::CEE_NEWARR ||
                    cInstr->m_opcode == rewriter::OPCODE::CEE_INITOBJ)
                {
                    const auto typeInfo = info::TypeInfo::GetTypeInfo(metadataImport, (mdTypeRef)cInstr->m_Arg32);
                    stream << "  | " << ToString(typeInfo.Name);
                }
                else if (cInstr->m_opcode == rewriter::OPCODE::CEE_LDSTR)
                {
                    WCHAR str[1024];
                    ULONG strLength;
                    auto hr = metadataImport->GetUserString((mdString)cInstr->m_Arg32, str, 1024, &strLength);
                    if (SUCCEEDED(hr))
                    {
                        stream << "  | \"" << util::ToString(util::wstring(str, strLength)) << "\"";
                    }
                }
            }
            else if (cInstr->m_Arg64 != 0)
            {
                stream << " " << cInstr->m_Arg64;
            }

            stream << std::endl;

            if (ehCount > 0)
            {
                for (auto i = 0; i < ehCount; i++)
                {
                    if (ehPointer[i].m_pHandlerEnd == cInstr)
                    {
                        indent--;
                        stream << createIndent(indent) << "}" << std::endl;
                    }
                }
            }
        }

        return util::ToWSTRING(stream.str());
	}
	catch (const std::bad_cast& c)
	{
        std::cout << c.what() << std::endl;
        return ""_W;
	}
}