//
#pragma once

#include "Recluse/Types.hpp"

#include "Recluse/Memory/Allocator.hpp"

#include <vector>
#include <map>

namespace Recluse {


struct BuddyBlock 
{
    U64     memSzBytes;     // Likely sizes of 64 KB
    U64     addressBytes;    // Offset of the buddy block in virtual memory.
    U64     blockId;        // block id.
};

// Buddy allocator strategy implementation.
//
class RecluseFramework_PUBLIC_API BuddyStrategy
{
public:

    typedef Allocation BlockAllocation;

    BuddyStrategy()
        : m_maxOrder(0) 
    { }

    ResultCode onInitialize(Allocator<BuddyStrategy>* super);
    ResultCode onAllocate(Allocator<BuddyStrategy>* super, Allocation* pOutput, U64 requestSz, U16 alignment);
    ResultCode onFree(Allocator<BuddyStrategy>* super, Allocation* pOutput);
    ResultCode onReset(Allocator<BuddyStrategy>* super);
    ResultCode onCleanUp(Allocator<BuddyStrategy>* super);

    ResultCode onRebase(Allocator<BuddyStrategy>* super, UPtr newAddress, U64 sizeBytes)
    {
        return RecluseResult_NoImpl;
    }

    inline BlockAllocation makeBlockAllocation(U64 blockAddress, U64 blockSzBytes)
    {
        return { blockAddress, blockSzBytes };
    }

private:

    std::vector<std::vector<BuddyBlock>>        m_freeList;
    std::map<SizeT, BlockAllocation>            m_allocatedBlocks;
    U32                                         m_maxOrder;
};

typedef Allocator<BuddyStrategy> BuddyAllocator;
} // Recluse