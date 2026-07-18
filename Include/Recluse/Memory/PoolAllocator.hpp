//

#pragma once

#include "Recluse/Memory/Allocator.hpp"
#include "Recluse/Memory/MemoryCommon.hpp"

namespace Recluse {


class RecluseFramework_PUBLIC_API PoolAllocator : public Allocator
{
public:
    PoolAllocator() : m_root(nullptr)
        { }

    virtual ~PoolAllocator();

    ResultCode onInitialize() override
    {
        return RecluseResult_NoImpl;
    }

    ResultCode onCleanUp() override
    {
        return RecluseResult_NoImpl;
    }

protected:
    ResultCode onAllocate(Allocation* output, U64 requestSizeBytes, U16 alignment) override;
    ResultCode onFree(Allocation* output) override;

    ResultCode onReset() override;
private:
    struct BlockNode
    {
        struct BlockNode*   next;
        UPtr                baseAddress;
        U32                 sizeBytes;
        U32                 pad0;
    };

    BlockNode* allocateBlock(U32 sizeBytes)
    {
        if (!m_root)
        {
            BlockNode* base = (BlockNode*)getBaseAddr();
            new (base) BlockNode();
            base->baseAddress = sizeof(BlockNode);
            base->next = nullptr;
            base->sizeBytes = sizeBytes;
        }
    }

    struct BlockNode* m_root;
};
} // Recluse