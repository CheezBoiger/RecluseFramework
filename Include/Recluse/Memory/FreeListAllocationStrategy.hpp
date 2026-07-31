//
#pragma once

#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"

#include <list>

namespace Recluse {


class FreeListStrategy
{
private:

    struct MemBlock 
    {
        U64         addressOffsetBytes;
        U64         blockSizeBytes;
    };

public:

    FreeListStrategy()
        : m_memBlockStart(nullptr) { }

    ~FreeListStrategy() 
    {
        m_memBlocks.clear();
    }

    ResultCode onInitialize(Allocator<FreeListStrategy>* super);
    ResultCode onAllocate(Allocator<FreeListStrategy>* super, Allocation* pOutput, U64 requestSz, U16 alignment);
    ResultCode onFree(Allocator<FreeListStrategy>* super, Allocation* pOutput);
    
    ResultCode onReset(Allocator<FreeListStrategy>* super);
    ResultCode onCleanUp(Allocator<FreeListStrategy>* super);

    ResultCode onRebase(Allocator<FreeListStrategy>* super, UPtr newAddress, U64 sizeBytes)
    {
        return RecluseResult_NoImpl;
    }

private:
    
    MemBlock*           m_memBlockStart;
    std::list<MemBlock> m_memBlocks;
};
} // Recluse