//
#include <Recluse/Memory/MemoryPool.hpp>
#include <Recluse/Memory/MemoryScan.hpp>
#include <Recluse/Memory/MemoryCommon.hpp>
#include <Recluse/Math/MathCommons.hpp>

#include <Recluse/Messaging.hpp>

#include <stdlib.h>

namespace Recluse {


void MemoryPool::copy(MemoryPool* dst, U64 dstOffset, MemoryPool* src, U64 srcOffset, U64 sizeBytes)
{
    R_ASSERT_FORMAT(dst && src, "Either src or dst memory pools are nullptr!");
    R_ASSERT_FORMAT(sizeBytes > 0, "Requested 0 bytes to copy!");

    const UPtr dstMemoryBase      = dst->getBaseAddress();
    const UPtr srcMemoryBase      = src->getBaseAddress();
    const UPtr srcMemoryOffset    = srcMemoryBase + srcOffset;
    UPtr dstMemoryOffset          = dstMemoryBase + dstOffset;
    memcpy((void*)dstMemoryOffset, (const void*)srcMemoryOffset, sizeBytes);
}


void MemoryPool::copy(void* dst, U64 dstOffset, void* src, U64 srcOffset, U64 sizeBytes)
{
    R_ASSERT_FORMAT(dst && src, "Either src or dst memory pools are nullptr!");
    R_ASSERT_FORMAT(sizeBytes > 0, "Requested 0 bytes to copy!");

    const UPtr dstMemoryBase      = (U64)dst;
    const UPtr srcMemoryBase      = (U64)src;
    const UPtr srcMemoryOffset    = srcMemoryBase + srcOffset;
    UPtr dstMemoryOffset          = dstMemoryBase + dstOffset;
    memcpy((void*)dstMemoryOffset, (const void*)srcMemoryOffset, sizeBytes);
}


void MemoryPool::copy(UPtr dst, U64 dstOffset, UPtr src, U64 srcOffset, U64 sizeBytes)
{
    R_ASSERT_FORMAT(dst && src, "Either src or dst memory pools are nullptr!");
    R_ASSERT_FORMAT(sizeBytes > 0, "Requested 0 bytes to copy!");

    const UPtr dstMemoryBase      = dst;
    const UPtr srcMemoryBase      = src;
    const UPtr srcMemoryOffset    = srcMemoryBase + srcOffset;
    UPtr dstMemoryOffset          = dstMemoryBase + dstOffset;
    memcpy((void*)dstMemoryOffset, (const void*)srcMemoryOffset, sizeBytes);
}


void* MemoryPool::pageAlignedMalloc(U64 sizeBytes, U32 pageSize)
{
    size_t totalSizeBytes = sizeBytes + pageSize + sizeof(void*);

    void* rawPtr = malloc(totalSizeBytes);

    if (!rawPtr) return nullptr;
    
    UPtr unalignedAddress = (UPtr)rawPtr + sizeof(void*);
    UPtr alignedAddress = Recluse::align(unalignedAddress, pageSize);

    ((void**)alignedAddress)[-1] = rawPtr;
    return (void*)alignedAddress;
}


void MemoryPool::freePagedAlignedMalloc(void* ptr)
{
    if (!ptr) return;

    void* rawPtr = ((void**)ptr)[-1];
    free(rawPtr);
}


MemoryPool::MemoryPool(U64 szBytes, U64 pageSz)
    : m_baseAddr(0ull)
    , m_pageSzBytes(pageSz)
    , m_totalSzBytes(0ull)
    , m_pScanStart(nullptr)
    , m_flags(0)     // Start with no malloc, if we are just empty initializing this pool.
{
    preAllocate(szBytes, pageSz);
}


MemoryPool::MemoryPool(void* ptr, U64 szBytes, U64 pageSz)
    : m_baseAddr(0ull)
    , m_pageSzBytes(pageSz)
    , m_totalSzBytes(0ull)
    , m_pScanStart(nullptr)
    , m_flags(0)
{
    m_baseAddr = (UPtr)ptr;
    m_pageSzBytes = pageSz;
    m_totalSzBytes = szBytes;
}


void MemoryPool::addScanner(MemoryScanner* scanner)
{
    MemScanNodes* trav = m_pScanStart;
    if (!m_pScanStart) 
    {
        m_pScanStart = new MemScanNodes(scanner);
        return;
    }

    MemScanNodes* node = new MemScanNodes(scanner);
    
    while (trav->next) 
    {
        trav = trav->next;
    }

    trav->next = node;
}


MemoryPool::~MemoryPool()
{
    release();
}


void MemoryPool::preAllocate(U64 szBytes, U64 pageSz)
{
    U64 allocationSizeBytes = szBytes;

    if (szBytes == 0ull || isAllocated())
    {
        // We don't have requested size bytes to allocate. This pre-allocation will be ignored.
        R_WARN
            (
                "MemoryPool", 
                "Memory pool is either already allocated, or 0 size bytes was passed to preAllocation."
                " Be sure to either call release(), or check if the correct behavior was expected."
            );
        return;
    }

    if (pageSz > 0)
    {
        m_baseAddr = (UPtr)pageAlignedMalloc(allocationSizeBytes, pageSz);
        m_flags |= IsPaged;
    }
    else
        m_baseAddr      = (UPtr)malloc(allocationSizeBytes);

    m_pageSzBytes   = pageSz;
    m_totalSzBytes  = allocationSizeBytes;
    m_flags         |= IsMalloc;
}


void MemoryPool::clear(U32 defaultValue)
{
    if (m_totalSzBytes == 0ull)
    {
        return;
    }
    // Set the memory pool to a default value, this is our clear!
    memset((void*)m_baseAddr, defaultValue, m_totalSzBytes);
}


void MemoryPool::reset()
{
    clear();
}


void MemoryPool::release()
{
    // Broadcast to observer scanners.
    MemScanNodes* trav = m_pScanStart;

    while (trav)
    {
        trav->pScanner->scanMemoryLeaks(this);
        trav = trav->next;
    }

    if (m_baseAddr && (m_flags & IsMalloc))
    {
        // Free the base address, and since it is malloc'ed, we need to point the address back to zero, to let the pool know
        // we no longer have memory attached.
        if (m_flags & IsPaged)
            freePagedAlignedMalloc((void*)m_baseAddr);
        else
            free((void*)m_baseAddr);

        m_flags = removeBitMask(m_flags, (IsMalloc | IsPaged));
    }

    m_baseAddr = 0;
}


Bool MemoryPool::resize(U64 newSizeBytes, U64 pageSize)
{
    if (m_baseAddr == 0)
        return false;
    R_ASSERT_FORMAT(newSizeBytes != 0, "New size for memory pool resize, should not be 0! Ignoring...");
    if (newSizeBytes == 0)
        return false;

    U64 sizeToCopyBytes = Math::minimum(newSizeBytes, m_totalSzBytes);

    void* newMemoryBase = nullptr;
    if (m_flags & IsPaged)
        newMemoryBase = pageAlignedMalloc(newSizeBytes, pageSize);
    else
        newMemoryBase = malloc(newSizeBytes);

    copy((UPtr)newMemoryBase, 0, m_baseAddr, 0,  sizeToCopyBytes);
    
    // Release the base address, then use the new memory base.
    release();

    m_baseAddr = (UPtr)newMemoryBase;

    m_flags |= IsMalloc;
    if (pageSize > 0) m_flags |= IsPaged;

    m_totalSzBytes = newSizeBytes;
    m_pageSzBytes = pageSize;

    return true;
}
} // Recluse