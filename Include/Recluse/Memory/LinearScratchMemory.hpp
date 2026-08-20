//
#pragma once

#include <Recluse/Memory/LinearAllocationStrategy.hpp>
#include <Recluse/Memory/MemoryPool.hpp>


namespace Recluse {


// Quick scratch memory allocator, uses the linear allocation strategy to 
// suballocate quick blocks for user compute.
// defaultSizeBytes is the current max size of the memory heap to allocate from.
// dynamic is whether the memory heap can resize, or reallocate more memory if out. This invalidates previous pointers if 
// dynamic is true. If dynamic is false, then the memory heap will not reallocate more memory and will return nullptr if out of memory.
// pageSz is the page size, for paged memory heap allocations. Usually, should be 0, unless a massive heap is needed.
template<U32 defaultSizeBytes, Bool dynamic = false, U32 pageSz = 0ull>
class LinearScratchMemory
{
public:
    
    LinearScratchMemory(uint sizeBytes = defaultSizeBytes, uint pageSize = pageSz)
    {
        initialize(sizeBytes, pageSize);
    }

    ~LinearScratchMemory()
    {
        destroy();
    }

    // Allocate and cast to Type. 
    // \param arrayCount The number of Type objects to allocate as an array. 1 is default. Should not be 0.
    template<typename Type>
    Type* allocate(U32 arrayCount = 1u, U16 alignment = 0u)
    {
        R_ASSERT_FORMAT(arrayCount > 0, "Incorrect size passed to allocate.");
        if (arrayCount == 0) return nullptr;
        Type* ptr = (Type*)allocator->allocate(sizeof(Type) * arrayCount, alignment);
        if constexpr (dynamic)
        {
            if (allocator->getLastError() == RecluseResult_OutOfMemory)
            {
                // Resize with double to original, plus the requested size.
                Bool success = resize(static_cast<uint>(memArena.getTotalSizeBytes() * 2 + sizeof(Type)) * arrayCount, static_cast<uint>(memArena.getPageSizeBytes()));
                if (success)
                    ptr = (Type*)allocator->allocate(sizeof(Type) * arrayCount, alignment);
            }
        }
        return ptr;
    }

    void* allocateRaw(UPtr sizeBytes, U16 alignment = 0u)
    {
        R_ASSERT(alignment > 0 && sizeBytes > 0);
        void* ptr = (void*)allocator->allocate(sizeBytes, alignment);
        if constexpr (dynamic)
        {
            if (allocator->getLastError() == RecluseResult_OutOfMemory)
            {
                // Resize with double to original, plus the requested size.
                Bool success = resize(static_cast<uint>(memArena.getTotalSizeBytes() * 2 + sizeBytes), static_cast<uint>(memArena.getPageSizeBytes()));
                if (success)
                    ptr = (void*)allocator->allocate(sizeBytes, alignment);
            }
        }
        return ptr;
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
        return 0;
    }

    UPtr getBaseAddress() const
    {
        return memArena.getBaseAddress() + sizeof(LinearAllocator);
    }

    Bool resize(uint newSize, uint pageSize)
    {
        memArena.resize(newSize, pageSize);
        allocator = (LinearAllocator*)memArena.getBaseAddress();
        ResultCode result = allocator->rebase(memArena.getPtrAddressAt(sizeof(LinearAllocator)), memArena.getTotalSizeBytes() - sizeof(LinearAllocator));
        return (result == RecluseResult_Ok);
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