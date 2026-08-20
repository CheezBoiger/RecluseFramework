//
#pragma once

#include <Recluse/Types.hpp>

#include <Recluse/Memory/Allocator.hpp>

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
class BuddyStrategy
{
public:

    typedef Allocation BlockAllocation;

    RecluseFramework_PUBLIC_API BuddyStrategy()
        : m_maxOrder(0) 
    { }

    RecluseFramework_PUBLIC_API ResultCode onInitialize(Allocator<BuddyStrategy>* super);
    RecluseFramework_PUBLIC_API ResultCode onAllocate(Allocator<BuddyStrategy>* super, Allocation* pOutput, U64 requestSz, U16 alignment);
    RecluseFramework_PUBLIC_API ResultCode onFree(Allocator<BuddyStrategy>* super, Allocation* pOutput);
    RecluseFramework_PUBLIC_API ResultCode onReset(Allocator<BuddyStrategy>* super);
    RecluseFramework_PUBLIC_API ResultCode onCleanUp(Allocator<BuddyStrategy>* super);

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