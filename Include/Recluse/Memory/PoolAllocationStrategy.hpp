//

#pragma once

#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"

namespace Recluse {


class RecluseFramework_PUBLIC_API PoolStrategy
{
public:
    PoolStrategy() : m_root(nullptr)
        { }

    ~PoolStrategy();

    ResultCode onInitialize()
    {
        return RecluseResult_NoImpl;
    }

    ResultCode onCleanUp()
    {
        return RecluseResult_NoImpl;
    }

protected:
    ResultCode onAllocate(Allocation* output, U64 requestSizeBytes, U16 alignment);
    ResultCode onFree(Allocation* output);

    ResultCode onReset();
private:
    struct BlockNode
    {
        struct BlockNode*   next;
        UPtr                baseAddress;
        U32                 sizeBytes;
        U32                 pad0;
    };

    BlockNode* allocateBlock(Allocator<PoolStrategy>* super, U32 sizeBytes)
    {
        if (!m_root)
        {
            BlockNode* base = (BlockNode*)super->getBaseAddress();
            new (base) BlockNode();
            base->baseAddress = sizeof(BlockNode);
            base->next = nullptr;
            base->sizeBytes = sizeBytes;
        }
    }

    ResultCode onRebase(Allocator<PoolStrategy>* super, UPtr newAddress, U64 sizeBytes)
    {
        return RecluseResult_NoImpl;
    }

    struct BlockNode* m_root;
};
} // Recluse