//
#pragma once

#include <Recluse/Memory/Allocator.hpp>
#include <Recluse/Memory/MemoryCommon.hpp>
#include <Recluse/Math/MathCommons.hpp>

#include <Recluse/Messaging.hpp>

namespace Recluse {


// Stack allocator, or linear allocator, which handles 
// temporary data to be used briefly.
class RecluseFramework_PUBLIC_API LinearStrategy
{
public:
    LinearStrategy()
        : m_top(0ull) { }

    ResultCode onInitialize(Allocator<LinearStrategy>* super)
    {
        m_top = super->getBaseAddress();
        return RecluseResult_Ok;
    }

    ResultCode onAllocate(Allocator<LinearStrategy>* super, Allocation* pOutput, U64 requestSz, U16 alignment)
    {
        R_ASSERT(Math::isPowerOf2(alignment));
        UPtr neededSzBytes   = requestSz + alignOffset(m_top, alignment);
        U64 totalSzBytes     = super->getTotalSizeBytes();
        UPtr szAddr          = super->getBaseAddress() + totalSzBytes;
        UPtr endAddr         = m_top + neededSzBytes;
    
        if (endAddr >= szAddr) 
        {
            return RecluseResult_OutOfMemory;
        }

        pOutput->baseAddress        = align(m_top, alignment);
        pOutput->sizeBytes          = neededSzBytes;
        m_top                       = endAddr;

        return RecluseResult_Ok;
    }

    UPtr getTop() const 
    {
        return m_top;
    }

    ResultCode onReset(Allocator<LinearStrategy>* super)
    { 
        m_top = super->getBaseAddress();
        return RecluseResult_Ok;
    }

    ResultCode onCleanUp(Allocator<LinearStrategy>* super)
    {   
        // Nothing to clean
        return RecluseResult_Ok;
    }

    ResultCode onFree(Allocator<LinearStrategy>* super, Allocation* pOutput)
    {
        R_ASSERT(false, "Linear allocator doesn't allow freeing!");
        return RecluseResult_NoImpl;
    }

    ResultCode onRebase(Allocator<LinearStrategy>* super, UPtr newAddress, U64 sizeBytes)
    {
        // super still holds the original base address, so get the offset of it.
        UPtr offset = m_top - super->getBaseAddress();

        // Should be the minimum between these two, especially if the new size is now smaller.
        offset = Math::minimum(offset, sizeBytes);

        m_top = newAddress + offset;
        return RecluseResult_Ok;
    }

private:
    UPtr m_top;
};

typedef Allocator<LinearStrategy> LinearAllocator;
} // Recluse