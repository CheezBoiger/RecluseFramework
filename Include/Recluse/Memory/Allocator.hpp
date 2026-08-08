// 
#ifndef RECLUSE_ALLOCATOR_HPP
#define RECLUSE_ALLOCATOR_HPP
#pragma once

#include <Recluse/Types.hpp>
#include <Recluse/Memory/MemoryCommon.hpp>
#include <Recluse/Messaging.hpp>

#include <RecluseFramework_exports.hpp>

namespace Recluse {

class MemoryPool;


//! Recluse allocation struct. Contains info of the suballocation for the requested data.
typedef struct Allocation 
{
    UPtr    baseAddress;        //< Base address/ starting address of the allocated object.
    U64     sizeBytes;          //< The size of the allocated memory that represents this object.
} *PAllocation, &RAllocation;

//! Recluse Allocator class. This is the class that is used for defining multiple allocation
//! data structures using a provided strategy.
//! 
//! Required declarations and implementations for the strategy to work:
//! 
//!     onInitialize(Allocator<Strategy>* super)
//!     onAllocate(Allocator<Strategy>* super, Allocation* output, U64 requestSizeBytes, U16 alignment)
//!     onFree(Allocator<Strategy>* super, Allocation* input)
//!     onReset(Allocator<Strategy>* super)
//!     onRebase(Allocator<Strategy>* super)
//!     onCleanUp(Allocator<Strategy>* super)
//! 
template<typename Strategy>
class Allocator
{
public:

    Allocator(UPtr basePtr = 0ull, U64 sizeBytes = 0ull) 
        : m_totalSizeBytes(sizeBytes)
        , m_pMemoryBaseAddr(basePtr)
        , m_initialized(false) { }

    //! Allocator mem size and page size (usually 4kb). 
    void initialize(UPtr pBasePtr, U64 sizeBytes) 
    {
        m_totalSizeBytes    = sizeBytes;
        m_pMemoryBaseAddr   = pBasePtr;
        ResultCode result = m_strategy.onInitialize(this);
        if (result == RecluseResult_Ok)
            m_initialized = true;
        setLastError(result);
    }

    //! Allocation requirements.
    UPtr allocate(U64 requestSz, U16 alignment) 
    {
        Allocation allocation = { };
        ResultCode err = m_strategy.onAllocate(this, &allocation, requestSz, alignment);
        setLastError(err);
        return allocation.baseAddress;
    }

    void free(UPtr ptr) 
    {
        Allocation alloc = { };
        alloc.baseAddress = ptr;
        alloc.sizeBytes = ~0;
        ResultCode err = m_strategy.onFree(this, &alloc);
        setLastError(err);
    }


    // Reset the allocator. This is more colloquially known as Clear().
    void reset() 
    {
        ResultCode result = m_strategy.onReset(this);
        setLastError(result);
    }

    void cleanUp() 
    {
        ResultCode result = m_strategy.onCleanUp(this);
        m_totalSizeBytes    = 0;
        m_pMemoryBaseAddr   = 0ull;
        
        setLastError(result);
        if (result == RecluseResult_Ok)
            m_initialized = false;
    }

    ResultCode rebase(UPtr newAddress, U64 sizeBytes)
    {
        if (!m_initialized)
        {
            setLastError(RecluseResult_Failed);
            return getLastError();
        }
        ResultCode result = m_strategy.onRebase(this, newAddress, sizeBytes);
        if (result == RecluseResult_Ok)
        {
            // allocator is rebased.
            m_totalSizeBytes = sizeBytes;
            m_pMemoryBaseAddr = newAddress;
        }
        setLastError(result);
        return result;
    }

    UPtr getBaseAddress() const { return m_pMemoryBaseAddr; }
    U64 getTotalSizeBytes() const { return m_totalSizeBytes; }
    ResultCode getLastError() const { return m_lastError; }

private:
    // A readable last error function, it is not really needed unless you manually override the last error.
    void setLastError(ResultCode err) { m_lastError = err; }
    Strategy m_strategy;

    U64     m_totalSizeBytes;
    UPtr    m_pMemoryBaseAddr;
    ResultCode m_lastError = RecluseResult_Ok;
    Bool    m_initialized;
};


class MallocAllocator
{
public:
    MallocAllocator() { }
    virtual ~MallocAllocator() { }

    ResultCode onInitialize()
    { 
        return RecluseResult_Ok; 
    }

    ResultCode onAllocate(Allocation* pOutput, U64 requestSz, U16 alignment)
    {
        U64 offset = alignment - 1 + sizeof(void*);
        U64 neededSzBytes = requestSz + offset;
        void* ptr = malloc(neededSzBytes);
        void** ptrr = (void**)(((UPtr)(ptr) + offset) & ~(alignment - 1));
        ptrr[-1] = ptr;
        pOutput->baseAddress = (U64)(void*)ptrr;
        pOutput->sizeBytes = requestSz;
        return RecluseResult_Ok;
    }

    ResultCode onFree(Allocation* pOutput)
    {
        void** ptrr = (void**)pOutput->baseAddress;
        ::free(ptrr[-1]);
        return RecluseResult_Ok;
    }

    ResultCode onReset() { return RecluseResult_Ok; }
    ResultCode onCleanUp() { return RecluseResult_Ok; }
};
} // Recluse


// Operator overload for placement new.
// This will allow us to overload the placement new operator in c++, to utilize our custom memory allocations.:
// ex. 
//          Object* pObj = new (allocator) Object();
//
template<typename Strategy>
void*   operator new (size_t sizeBytes, Recluse::Allocator<Strategy>* alloc)
{
    R_ASSERT(alloc != NULL);
    return (void*)alloc->allocate(sizeBytes, Recluse::pointerSizeBytes());
}

template<typename Strategy>
void* operator new[] (size_t bytes, Recluse::Allocator<Strategy>* alloc)
{
    R_ASSERT(alloc != NULL);
    return (void*)alloc->allocate(bytes, Recluse::pointerSizeBytes());
}

// Operator overload for deleting allocated pointers.
// This is a helpful function, instead of having to all individually the object allocator, and performing a bunch of
// stuff...
//
template<typename Strategy>
void    operator delete (void* ptr, Recluse::Allocator<Strategy>* alloc)
{
    R_ASSERT(alloc != NULL);

    alloc->free((Recluse::UPtr)ptr);

    R_DEBUG_WRAP(Recluse::ResultCode err = alloc->getLastError());
    R_DEBUG_WRAP(R_ASSERT(err == Recluse::RecluseResult_Ok));
}

template<typename Strategy>
void    operator delete[] (void* ptr, Recluse::Allocator<Strategy>* alloc)
{
    alloc->free((Recluse::UPtr)ptr);
}
#endif // RECLUSE_ALLOCATOR_HPP