//
#pragma once

#include "Recluse/Types.hpp"
#include "Recluse/Threading/Threading.hpp"
#include "Recluse/Threading/Sema.hpp"

#include "RecluseFramework_exports.hpp"

#include <vector>
#include <list>
#include <functional>

namespace Recluse {


typedef std::function<void()> ThreadTask;

// ThreadPool is a structure that handles the concurrent execution of tasks, without needing to 
// re-create workers, and instead, re-use existing threads. The workers themselves will remain alive, 
// until signalled to stop, and all remaining enqueued tasks have been completed.
class ThreadPool 
{
public:
    enum Status 
    {
        Status_Idle,
        Status_Running,
        Status_Pause,
        Status_Stopping,
        Status_Stopped
    };

    enum Signal 
    {
        Signal_None,
        Signal_Stop = (1 << 0),
        Signal_Pause = (1<<1),
        Signal_Resume =(1<<2)
    };

    RecluseFramework_PUBLIC_API ThreadPool(U32 numWorkers = 2);
    RecluseFramework_PUBLIC_API ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    RecluseFramework_PUBLIC_API ThreadPool(ThreadPool&&) noexcept;
    RecluseFramework_PUBLIC_API ThreadPool& operator=(ThreadPool&&) noexcept;

    // Get the number of workers in the pool.
    uint getWorkerCount() const { return (uint)m_threadWorkers.size(); }

    // Submits a task to the pool, this will be picked up by a worker thread 
    // and completed.    
    template<typename F, typename... Args>
    ResultCode submitTask(F&& f, Args&&... args)
    {
        // Package the function and its arguments into a std::function<void()>
        ThreadTask job = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

        // Pass the bound task into your existing internal submission logic
        return submitTaskInternal(std::move(job));
    }
    
    // Starts up the pool of workers, which will run concurrently until stop() is called.
    RecluseFramework_PUBLIC_API void start();

    // Signals worker threads to stop, and finishes any remaining tasks in the pool.
    RecluseFramework_PUBLIC_API void stop();

    // Blocking wait until the thread pool is finished and idle.
    RecluseFramework_PUBLIC_API void waitIdle();

    //Bool isExecuting();
    
private:
    // The actual worker itself.
    struct Worker
    {
        static const uint kBadIndex = ~0;

        Worker(const CriticalSection& section = { }, ThreadPool* pool = nullptr, uint index = kBadIndex)
            : poolRef(pool)
            , section(section)
            , signals(0)
            , workerIndex(index)
            , status(Status_Stopped) { thread = { }; }

        Thread                      thread;
        volatile Status             status;
        uint                        workerIndex;
        CriticalSection::Reference  section;
        ThreadPool*                 poolRef;

        ThreadTask  nextTask();
        void        signal(Signal signal) { signals |= signal; }
        U32         getSignals() { return signals; }
        void        join();
        void        clearSignals() { signals = 0; }

    private:
        U32         signals;
    };

    static U32 threadEntryTask(void* payload);

    Worker* getWorkerData(uint index) { return &m_threadWorkers[index]; }

    RecluseFramework_PUBLIC_API ResultCode submitTaskInternal(ThreadTask job);

    // Tasks to complete, which are carried by worker threads.
    CriticalSection                     m_taskCs;

    std::list<ThreadTask>               m_jobTasks;
    std::vector<Worker>                 m_threadWorkers;
};
} // Recluse