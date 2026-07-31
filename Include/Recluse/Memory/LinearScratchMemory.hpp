//
#pragma once

#include <Recluse/Memory/LinearAllocationStrategy.hpp>
#include <Recluse/Memory/MemoryPool.hpp>


namespace Recluse {


template<U32 sizeBytes, U32 pageSz = 4096u, Bool dynamic = false>
class LinearScratchMemory
{
public:
    
    LinearScratchMemory()
    {
        initialize(sizeBytes, pageSz);
    }

    ~LinearScratchMemory()
    {
        destroy();
    }

    template<typename Type>
    Type* allocate(U32 arrayCount = 1u)
    {
        R_ASSERT(arrayCount > 0);
        if constexpr (dynamic)
        {
            Type* ptr = (Type*)allocator->allocate(sizeof(Type) * arrayCount, pointerSizeBytes());
            if (allocator->getLastError() == RecluseResult_OutOfMemory)
            {
                memArena.resize(memArena.getTotalSizeBytes() * 2, memArena.getPageSizeBytes());
                allocator = (LinearAllocator*)memArena.getBaseAddress();
                allocator->rebase(memArena.getPtrAddressAt(sizeof(LinearAllocator), memArena.getTotalSizeBytes() - sizeof(LinearAllocator)));
            }
            return ptr;
        }
        else
            return (Type*)allocator->allocate(sizeof(Type) * arrayCount, pointerSizeBytes());
    }

    void free(void* ptr)
    {
        allocator->free((UPtr)ptr);
    }

    void clear()
    {
        allocator->reset();
    }

    U32 getTotalAllocations() const 
    {
        return allocator->getTotalAllocations();
    }

private:

    void initialize(u64 newSizeBytes, u64 pageSize)
    {
        memArena.preAllocate(newSizeBytes + sizeof(LinearAllocator), pageSize);
        allocator = new ((void*)memArena.getBaseAddress()) LinearAllocator();
        allocator->initialize(memArena.getPtrAddressAt(sizeof(LinearAllocator)), memArena.getTotalSizeBytes() - sizeof(LinearAllocator));
    }

    void destroy()
    {
        allocator->cleanUp();
        memArena.clear();
        memArena.release();
    }

    MemoryArena memArena;
    LinearAllocator* allocator;
};
} // Recluse