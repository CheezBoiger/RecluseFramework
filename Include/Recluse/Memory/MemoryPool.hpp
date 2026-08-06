//
#ifndef RECLUSE_MEMORY_POOL_HPP
#define RECLUSE_MEMORY_POOL_HPP

#pragma once

#include <Recluse/Types.hpp>

#include <RecluseFramework_exports.hpp>

namespace Recluse {


template<typename Type, typename ...Arguments>
static Type* rlsMalloc(Arguments... args)
{
    return new Type(args...);
}


template<typename Type>
static Type* rlsMalloc(U64 count)
{
    return new Type[count];
}


template<typename Type>
static void rlsFree(Type* ptr)
{
    delete ptr;
}


template<typename Type>
static void rlsFreeArray(Type* ptr)
{
    delete[] ptr;
}


class MemoryScanner;


// Memory Pool consists of an allocated space that will be used for suballocations, or committed space.
// This arena is intended to be used with Allocators.
class RecluseFramework_PUBLIC_API MemoryPool 
{
public:
    // Memory pool allocation construction.
    MemoryPool(U64 szBytes = 0ull, U64 pageSize = 0ull);

    // Memory Pool to act as a handler to the range. Takes a pointer to the raw memory.
    // Memory can be malloc'ed or local, but won't be the responsibility of this pool to clean up.
    MemoryPool(void* ptr, U64 szBytes, U64 pagSz = 0ull);

    ~MemoryPool();
        
    // Get the starting address of the memory pool.
    UPtr                getBaseAddress() const { return m_baseAddr; }

    // Get the total size of the pool, in bytes.
    U64                 getTotalSizeBytes() const { return m_totalSzBytes; }

    // Get the page size used for the pool, in bytes.
    U64                 getPageSizeBytes() const { return m_pageSzBytes; }

    inline void*        getRawAddressAt(U64 sizeBytes) { return reinterpret_cast<void*>(m_baseAddr + sizeBytes); }
    inline UPtr         getPtrAddressAt(U64 sizeBytes) { return (m_baseAddr + sizeBytes); }

    // Add a memory leak scanner to the pool, to ensure there are no memory leaks.
    void                addScanner(MemoryScanner* scanner);

    // Pre allocates the memory pool.
    void                preAllocate(U64 szBytes, U64 pageSize = 0ull);

    // Resizes the pool, must already have been pre allocated. Returns true if the resize was successful, false if failed. If failed,
    // Will continue to keep the original memory pool size and allocation.
    Bool                resize(U64 newSizeBytes, U64 pageSize);

    // Clear the pool, wipe out the state and set to default value.
    // This does not free the pool memory! 
    // \param defaultValue The default value (in a byte) that will be used to fill the memory block.
    void                clear(U32 defaultValue = 0);

    // Is exactly clear, just another name for it.
    void                reset();

    // Free the pool memory! This will delete the memory!
    void                release();

    // Check if the memory pool is allocated, otherwise return false if pool is not.
    Bool                isAllocated() { return !!m_totalSzBytes; }

    // Copy contents from source memory pool, to destination memory pool.
    static void         copy(MemoryPool* dst, U64 dstOffset, MemoryPool* src, U64 srcOffset, U64 sizeBytes);
    static void         copy(void* dst, U64 dstOffset, void* src, U64 srcOffset, U64 sizeBytes);
    static void         copy(UPtr dst, U64 dstOffset, UPtr src, U64 srcOffset, U64 sizeBytes);

    // Perform a paged aligned allocation, should be used for full custom allocators and essential large heap allocations,
    // not for tiny ones!
    static void*        pageAlignedMalloc(U64 sizeBytes, U32 pageSize);

    // Free a paged aligned allocation.
    static void         freePagedAlignedMalloc(void* ptr);

private:
    enum {
        // If the allocation done was allocated via malloc. 
        IsMalloc = 1, 
        // If the allocation is paged.
        IsPaged = 2 
    };

    UPtr    m_baseAddr;
    U64     m_totalSzBytes;
    U64     m_pageSzBytes;
    U32     m_flags;

    struct MemScanNodes 
    {
        MemoryScanner*          pScanner;
        struct MemScanNodes*    next;

        MemScanNodes(MemoryScanner* scanner = nullptr, struct MemScanNodes* nextNode = nullptr)
            : pScanner(scanner)
            , next(nextNode)
        { }
    } *m_pScanStart;
};


// Name aliases
typedef MemoryPool MemoryArena;
typedef MemoryPool ScratchMem;
typedef MemoryPool MemoryHeap;
typedef ScratchMem ScratchMemory;

} // Recluse
#endif // RECLUSE_MEMORY_POOL_HPP