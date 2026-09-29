//
#pragma once

#include "Recluse/Types.hpp"

#include "RecluseFramework_exports.hpp"
#include "Recluse/Messaging.hpp"

#include <memory>

namespace Recluse {

// Thread function, used and called as the start routine for creating threads.
typedef U32(*ThreadFunction)(void*);

enum ThreadResultCode 
{
    ThreadResultCode_Unknown       = -999,
    ThreadResultCode_NotReady      = -2,
    ThreadResultCode_Failed        = -1,
    ThreadResultCode_Ok            =  0,
};


enum ThreadState 
{
    ThreadState_Stopped,
    ThreadState_Starting,
    ThreadState_Running,
    ThreadState_Suspended,
    ThreadState_Idle,
    ThreadState_Unknown
};

typedef void* ThreadPayload;

struct RecluseFramework_PUBLIC_API Thread 
{
    ThreadFunction      func;
    ThreadPayload       payload;
    SizeT               uid;
    void*               handle;
    U32                 resultCode;
    U32                 threadState;
};



typedef void* Mutex;
typedef void* Cond;
typedef void* Semaphore;


namespace MutexValue {

constexpr Mutex kNull = nullptr;

} // MutexVal

constexpr U64 kInfiniteMs = ~0ull;

RecluseFramework_PUBLIC_API R_OS_CALL ResultCode createThread(Thread* thread, ThreadFunction startRoutine);
RecluseFramework_PUBLIC_API R_OS_CALL ResultCode resumeThread(Thread* thread);
RecluseFramework_PUBLIC_API R_OS_CALL ResultCode stopThread(Thread* thread);
RecluseFramework_PUBLIC_API R_OS_CALL ResultCode detachThread(Thread* thread);
RecluseFramework_PUBLIC_API R_OS_CALL ResultCode joinThread(Thread* thread);
RecluseFramework_PUBLIC_API R_OS_CALL ResultCode killThread(Thread* thread);

RecluseFramework_PUBLIC_API R_OS_CALL Mutex   createMutex(const char* name = nullptr);
RecluseFramework_PUBLIC_API R_OS_CALL ResultCode lockMutex(Mutex mutex, U64 waitMs = kInfiniteMs);
RecluseFramework_PUBLIC_API R_OS_CALL ResultCode unlockMutex(Mutex mutex);
RecluseFramework_PUBLIC_API R_OS_CALL ResultCode waitMutex(Mutex mutex, U64 waitTimeMs);
RecluseFramework_PUBLIC_API R_OS_CALL ResultCode destroyMutex(Mutex mutex);
RecluseFramework_PUBLIC_API R_OS_CALL ResultCode tryLockMutex(Mutex mutex);

RecluseFramework_PUBLIC_API R_OS_CALL i32       fetchAdd(uptr ptr, i32 arg);
RecluseFramework_PUBLIC_API R_OS_CALL i32       fetchSub(uptr ptr, i32 arg);
RecluseFramework_PUBLIC_API R_OS_CALL U64       getMainThreadId();
RecluseFramework_PUBLIC_API R_OS_CALL U64       getCurrentThreadId();

RecluseFramework_PUBLIC_API R_OS_CALL Semaphore  createSemaphore(const char* name = nullptr);
RecluseFramework_PUBLIC_API R_OS_CALL ResultCode    destroySemaphore(Semaphore sema);
RecluseFramework_PUBLIC_API R_OS_CALL ResultCode    signalSemaphore(Semaphore sema);
RecluseFramework_PUBLIC_API R_OS_CALL ResultCode    waitSemaphore(Semaphore sema);

RecluseFramework_PUBLIC_API R_OS_CALL U64    compareExchange(I64* dest, I64 ex, I64 comp);
RecluseFramework_PUBLIC_API R_OS_CALL I16    compareExchange(I16* dest, I16 ex, I16 comp);
RecluseFramework_PUBLIC_API R_OS_CALL U128   compareExchange(U128* dest, U128 ex, U128 comp);

RecluseFramework_PUBLIC_API R_OS_CALL u32 testAndSet(volatile uptr ptr, U32 offset);

// Causes this thread to sleep for some milliseconds.
RecluseFramework_PUBLIC_API R_OS_CALL ResultCode    sleep(U64 milliseconds);

// C++ RAII locking mechanism within a scope.
// Intended for scope locking mutexes.
class RecluseFramework_PUBLIC_API ScopedLock 
{
public:
    ScopedLock(Mutex mutex) 
        : m_mut(mutex)
    {
        lockMutex(m_mut); 
    }

    ~ScopedLock() 
    {
        unlockMutex(m_mut); 
    }
private:
    Mutex m_mut;

    ScopedLock(const ScopedLock&)   = delete;
    ScopedLock(ScopedLock&&)        = delete;
};

class MutexGuard
{
public:
    MutexGuard(const char* debugName = nullptr) { m_mutex = createMutex(debugName); }
    ~MutexGuard() { destroyMutex(m_mutex); }

    MutexGuard(const MutexGuard&) = delete;
    MutexGuard& operator=(const MutexGuard&) = delete;

    MutexGuard(MutexGuard&& other) noexcept : m_mutex(other.m_mutex) { other.m_mutex = nullptr; }
    MutexGuard& operator=(MutexGuard&& other) noexcept { m_mutex = other.m_mutex; other.m_mutex = nullptr; return *this; }

    operator Mutex() const { return m_mutex; }
    Mutex* operator*() { return &m_mutex; }
    Mutex& operator&() { return m_mutex; }
private:
    Mutex m_mutex;
};


// Critical section is a way to internally handle concurrency. 
// Microsoft may attempt to handle critical code sections in user-mode in order to 
// prevent kernel level calls, which would be slower. Linux simply uses mutexes 
// and spinlocks, which can provide about similar performance as well.
class RecluseFramework_PUBLIC_API CriticalSection
{
public:
    typedef void* Handle;

    class RecluseFramework_PUBLIC_API Reference
    {
    public:
        Reference(const CriticalSection& section = {})
            : m_handleRef(section.m_section) { }

        Reference(const Reference& reference)
            : m_handleRef(reference.m_handleRef) { }

        R_OS_CALL ResultCode enter();

        // Returns Ok if the critical section is owned by this thread. Returns fail, if 
        // the attempt fails.
        R_OS_CALL ResultCode tryEnter();

        R_OS_CALL ResultCode leave();
    private:
        Handle m_handleRef;
    };

    CriticalSection()
        : m_section(nullptr)
    { }
    ~CriticalSection()
    { if (m_section) release(); m_section = nullptr; }

    CriticalSection(const CriticalSection&) = delete;
    CriticalSection& operator=(const CriticalSection&&) = delete;

    CriticalSection(CriticalSection&& other) noexcept
        : m_section(std::move(other.m_section))
        { other.m_section = nullptr; }

    CriticalSection& operator=(CriticalSection&& other) noexcept
    {
        m_section = other.m_section;
        other.m_section = nullptr;
        return *this;
    }

    ResultCode initialize();
    ResultCode release();

    R_OS_CALL ResultCode enter();

    // Returns Ok if the critical section is owned by this thread. Returns fail, if 
    // the attempt fails.
    R_OS_CALL ResultCode tryEnter();

    R_OS_CALL ResultCode leave();

    Bool isInitialized() const { return !!m_section; }

private:
    Handle m_section;
};

// C++ RAII critical section mechanism used for handling enter and exit
// calls to the critical section. 
class RecluseFramework_PUBLIC_API ScopedCriticalSection
{
public:
    ScopedCriticalSection(const CriticalSection& cs)
        : m_cs(cs)
    {
        m_cs.enter();
    }

    ScopedCriticalSection(const CriticalSection::Reference& ref)
        : m_cs(ref)
    {
        m_cs.enter();
    }

    ~ScopedCriticalSection()
    {
        m_cs.leave();
    }
private:
    CriticalSection::Reference m_cs;
};


class RecluseFramework_PUBLIC_API CriticalSectionGuard : public CriticalSection
{
public:
    CriticalSectionGuard() { initialize(); }
    ~CriticalSectionGuard() { if (isInitialized()) release(); }
};


class RecluseFramework_PUBLIC_API SpinlockObject
{
public:
    SpinlockObject()
        : m_v(0)
    { }
    uptr operator()() volatile { return (uptr)&m_v; }
private:
    volatile i32 m_v;
};


class RecluseFramework_PUBLIC_API ScopedSpinlock
{
public:
    ScopedSpinlock(SpinlockObject& o)
        : m_o(o)
    {
        while (testAndSet(m_o(), 0));
    }

    ~ScopedSpinlock()
    {
        i32 v = fetchSub(m_o(), 1);
        R_ASSERT(v == 0);
    }

private:
    volatile SpinlockObject& m_o;
};
} // Recluse