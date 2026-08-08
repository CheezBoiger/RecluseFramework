//
#pragma once

#include <Recluse/Types.hpp>
#include <Recluse/Threading/ThreadPool.hpp>

#include <RecluseFramework_exports.hpp>

#include <functional>
#include <map>

namespace Recluse {

typedef std::function<ResultCode()> Task;
typedef U32 TaskPriority;

typedef U32 TaskTypeFlags;


// Task manager that runs and schedules tasks based on priority.
// Uses current thread pool to handle and run certain tasks.
class RecluseFramework_PUBLIC_API TaskManager
{
public:
    typedef U32 AsyncTaskId;

    TaskManager();
    ~TaskManager();

    ResultCode      initialize();
    ResultCode      cleanUp();

    // Push a task with the given priorities.
    // Likely want this to be pushed during update call, so that thread pool can take hold.
    // Parallel tasks are done when they are pushed with the same priority.
    // 0 is highest priority, with least priority values going up.
    ResultCode      pushTask(TaskPriority priority, Task task);

    // Asyncronous task, that does not run in parallel like the pushTask().
    // This task works without barriers, and should work separately.
    AsyncTaskId     asyncTask(Task task, ThreadPool* pool);
    void            waitForTask(AsyncTaskId taskId);

    // Dispatch all pushed tasks that were called with pushTask(). 
    // Ensure any data within scope, should be called with this manually in the scope of that data to be processed.
    // Failure to do so will result in undefined behaviour, likely a crash.
    ResultCode      dispatchTasks(ThreadPool* pool);
    void            clearTasks();

private:
    struct AsyncTask
    {
        Task task;
        Bool finished;
    };
    Mutex                                       m_tasksMutex;
    CriticalSection                             m_asyncCs;
    std::map<TaskPriority, std::vector<Task>>   m_tasks;
    std::map<AsyncTaskId, AsyncTask>            m_asyncTasks;
};


// Task process is a separate asyncronous process, that runs independent of the main thread.
// This would need to be used for anything that requires it's own independent execution.
class RecluseFramework_PUBLIC_API TaskProcess
{

public:
    enum Signal 
    {
        Signal_Notify,  //< Signal to notify the process in it's task.
        Signal_Stop,    //< Signal to stop the process.
        Signal_Pause,   //< Signal to pause the process.
        Signal_Resume   //< Signal to resume the process.
    };

    typedef ThreadFunction ProcessTask;
    typedef std::function<ResultCode(TaskProcess*)> OnProcessTask; 

    TaskProcess(ThreadPool* workerPool = nullptr, OnProcessTask onTask = nullptr, const char* processName = nullptr)
        : m_onTask(onTask)
        , m_threadPoolRef(workerPool)
        , m_processName(processName ? processName : "")
        , m_isRunning(false) { }

    ~TaskProcess() { }

    // Start the process.
    ResultCode                  start();

    // Push a task with the given priorities.
    // Likely want this to be pushed during update call, so that thread pool can take hold.
    // Parallel tasks are done when they are pushed with the same priority.
    // 0 is highest priority, with least priority values going up.
    ResultCode                  pushTask(TaskPriority priority, Task task) { return m_taskManager.pushTask(priority, task); }

    // Asyncronous task, that does not run in parallel like the pushTask().
    // This task works without barriers, and should work separately.
    TaskManager::AsyncTaskId    asyncTask(Task task) { return m_taskManager.asyncTask(task, m_threadPoolRef); }
    void                        waitForTask(TaskManager::AsyncTaskId taskId) { m_taskManager.waitForTask(taskId); }

    // Get the process task that is running on this Process.
    OnProcessTask               getOnProcessTask() { return m_onTask; }

    // Check if this process is running.
    Bool                        isRunning() const  { return m_isRunning; }

    // Signal to the process. Can be called by the main task.
    void                        signal(Signal signal = Signal_Notify);

    // Dispatch all pushed tasks that were called with pushTask(). 
    // Ensure any data within scope, should be called with this manually in the scope of that data to be processed.
    // Failure to do so will result in undefined behaviour, likely a crash.
    ResultCode                  dispatchTasks() { return m_taskManager.dispatchTasks(m_threadPoolRef); }
    void                        clearTasks() { m_taskManager.clearTasks(); }

    std::string                 getProcessName() const { return m_processName; }
    TaskManager&                getTaskManager() { return m_taskManager; }

    // Waits to join back with the caller thread. Will block the caller until this process is complete.
    // Will not attempt to join, if the process itself attempts to call this.
    void                        join();

private:

    TaskManager                                 m_taskManager;
    Bool                                        m_isRunning;
    Thread                                      m_thread;

    ProcessTask                                 m_mainTask;
    OnProcessTask                               m_onTask;
    ThreadPool*                                 m_threadPoolRef;
    std::string                                 m_processName;
};
} // Recluse