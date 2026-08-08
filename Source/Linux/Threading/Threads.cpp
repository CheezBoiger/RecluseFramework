//

#include <Recluse/Types.hpp>
#include <Recluse/Threading/Threading.hpp>
#include <pthread.h>

namespace Recluse {

// Native function handle for the unix pthreads create.
void* native_func_handle(void* args)
{
    // Must have passed the thread struct with data.
    if (!args) return reinterpret_cast<void*>(intptr_t(0));

    Thread* thread = reinterpret_cast<Thread*>(args);
    return reinterpret_cast<void*>(
        static_cast<intptr_t>(thread->func(thread->payload)));
}

ResultCode createThread(Thread* thread, ThreadFunction startRoutine)
{
    if (!thread)
    {
        R_ERROR("Unix", "Invalid thread input was passed to createThread. This is invalid.");
        return RecluseResult_InvalidArgs;
    }
    ResultCode result = RecluseResult_Ok;
    pthread_t threadId = 0;
    pthread_attr_t attr;

    // We definitely want someway to modify the stack size.
    ssize_t stackSize = -1;

    int err = pthread_attr_init(&attr);

    // Stack size can not be lower than this.
    if (stackSize < PTHREAD_STACK_MIN)
        stackSize = PTHREAD_STACK_MIN;

    pthread_attr_setstacksize(&attr, stackSize);
    
    err = pthread_create(&threadId, &attr, &native_func_handle, &thread);

    if (err != 0)
        result = RecluseResult_Failed;

    err = pthread_attr_destroy(&attr);

    return result;
}


Mutex createMutex(const char* name)
{

    pthread_mutex_t* native_mutex = nullptr;
    pthread_mutexattr_t mutex_attrib;
    
    int err = pthread_mutexattr_init(&mutex_attrib);

    // We are allocating the mutex type on the heap. should be destroyed later on.
    native_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));

    err = pthread_mutex_init(native_mutex, &mutex_attrib);

    pthread_mutexattr_destroy(&mutex_attrib);
    if (err != 0)
        return nullptr;

    return reinterpret_cast<Mutex>(native_mutex);
}


ResultCode joinThread(Thread* thread)
{
    if (!thread) return RecluseResult_InvalidArgs;

    intptr_t argo = 0;

    int err = pthread_join((pthread_t)thread->handle, (void**)&argo);

    if (err != 0)
        return RecluseResult_Failed;

    return static_cast<ResultCode>(argo);
}


ResultCode detachThread(Thread* thread)
{
    if (!thread) return RecluseResult_InvalidArgs;

    int err = pthread_detach(reinterpret_cast<pthread_t>(thread->handle));

    if (err != 0)
        return RecluseResult_Failed;

    return RecluseResult_Ok;
}


ResultCode destroyMutex(Mutex mutex)
{
    if (!mutex) return RecluseResult_InvalidArgs;

    pthread_mutex_t* native_mutex = reinterpret_cast<pthread_mutex_t*>(mutex);
    pthread_mutex_destroy(native_mutex);

    // Free the mutex that was allocated on the heap.
    free(native_mutex);
    
    return RecluseResult_Ok;
}


U64 getCurrentThreadId()
{
    pthread_t thread_id = pthread_self();
    return (U64)thread_id;
}


ResultCode lockMutex(Mutex mutex, U64 waitMs)
{
    int err = pthread_mutex_lock((pthread_mutex_t*)mutex);

    if (err != 0)
        return RecluseResult_Timeout;

    return RecluseResult_Ok;
}


ResultCode unlockMutex(Mutex mutex)
{
    int err = pthread_mutex_unlock((pthread_mutex_t*)mutex);

    if (err != 0)
        return RecluseResult_Failed;

    return RecluseResult_Ok;
}
} // Recluse

