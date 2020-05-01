// Copyright (c) .NET Foundation and contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include "cor.h"
#include "corprof.h"
#include <corhlpr.h>

namespace rewriter
{

    typedef enum
    {
#define OPDEF(c,s,pop,push,args,type,l,s1,s2,ctrl) c,
#include "opcode.def"
#undef OPDEF
        CEE_COUNT,
        CEE_SWITCH_ARG, // special internal instructions
    } OPCODE;

    struct ILInstr
    {
        ILInstr* m_pNext;
        ILInstr* m_pPrev;

        unsigned        m_opcode;
        unsigned        m_offset;

        union
        {
            ILInstr* m_pTarget;
            INT8        m_Arg8;
            INT16       m_Arg16;
            INT32       m_Arg32;
            INT64       m_Arg64;
        };
    };

    struct EHClause
    {
        CorExceptionFlag            m_Flags;
        ILInstr* m_pTryBegin;
        ILInstr* m_pTryEnd;
        ILInstr* m_pHandlerBegin;    // First instruction inside the handler
        ILInstr* m_pHandlerEnd;      // Last instruction inside the handler
        union
        {
            DWORD                   m_ClassToken;   // use for type-based exception handlers
            ILInstr* m_pFilter;      // use for filter-based exception handlers (COR_ILEXCEPTION_CLAUSE_FILTER is set)
        };
    };

    class ILRewriter
    {
    private:
        ICorProfilerInfo* m_pICorProfilerInfo;
        ICorProfilerFunctionControl* m_pICorProfilerFunctionControl;

        ModuleID    m_moduleId;
        mdToken     m_tkMethod;

        mdToken     m_tkLocalVarSig;
        unsigned    m_maxStack;
        unsigned    m_flags;
        bool        m_fGenerateTinyHeader;

        ILInstr m_IL; // Double linked list of all il instructions

        unsigned    m_nEH;
        EHClause* m_pEH;

        // Helper table for importing.  Sparse array that maps BYTE offset of beginning of an
        // instruction to that instruction's ILInstr*.  BYTE offsets that don't correspond
        // to the beginning of an instruction are mapped to NULL.
        ILInstr** m_pOffsetToInstr;
        unsigned    m_CodeSize;

        unsigned    m_nInstrs;

        BYTE* m_pOutputBuffer;

        IMethodMalloc* m_pIMethodMalloc;

    public:
        ILRewriter(ICorProfilerInfo* pICorProfilerInfo, ICorProfilerFunctionControl* pICorProfilerFunctionControl, ModuleID moduleID, mdToken tkMethod)
            : m_pICorProfilerInfo(pICorProfilerInfo), m_pICorProfilerFunctionControl(pICorProfilerFunctionControl),
            m_moduleId(moduleID), m_tkMethod(tkMethod), m_fGenerateTinyHeader(false),
            m_pEH(nullptr), m_pOffsetToInstr(nullptr), m_pOutputBuffer(nullptr), m_pIMethodMalloc(nullptr)
        {
            m_IL.m_pNext = &m_IL;
            m_IL.m_pPrev = &m_IL;

            m_nInstrs = 0;
        }

        ~ILRewriter()
        {
            ILInstr* p = m_IL.m_pNext;
            while (p != &m_IL)
            {
                ILInstr* t = p->m_pNext;
                delete p;
                p = t;
            }
            delete[] m_pEH;
            delete[] m_pOffsetToInstr;
            delete[] m_pOutputBuffer;

            if (m_pIMethodMalloc)
                m_pIMethodMalloc->Release();
        }

        /////////////////////////////////////////////////////////////////////////////////////////////////
        //
        // I M P O R T
        //
        ////////////////////////////////////////////////////////////////////////////////////////////////

        HRESULT Import();

        HRESULT ImportIL(LPCBYTE pIL);

        HRESULT ImportEH(const COR_ILMETHOD_SECT_EH* pILEH, unsigned nEH);

        ILInstr* NewILInstr();

        ILInstr* GetInstrFromOffset(unsigned offset);

        void InsertBefore(ILInstr* pWhere, ILInstr* pWhat);

        void InsertAfter(ILInstr* pWhere, ILInstr* pWhat);

        void AdjustState(ILInstr* pNewInstr);


        ILInstr* GetILList();

        /////////////////////////////////////////////////////////////////////////////////////////////////
        //
        // E X P O R T
        //
        ////////////////////////////////////////////////////////////////////////////////////////////////


        HRESULT Export();

        HRESULT SetILFunctionBody(unsigned size, LPBYTE pBody);

        LPBYTE AllocateILMemory(unsigned size);

        void DeallocateILMemory(LPBYTE pBody);

        void InitializeTiny();
        void SetTkLocalVarSig(mdToken tkLocalVarSig);
    };
}