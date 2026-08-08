//
#include <Recluse/System/TaskManager.hpp>

#include <Recluse/Messaging.hpp>

namespace Recluse {

TaskManager::TaskManager()
    : m_tasksMutex(nullptr)
    , m_asyncCs()
{
}

TaskManager::~TaskManager()
{
    cleanUp();
}


ResultCode TaskManager::initialize()
{
    m_tasksMutex = createMutex("TasksMutex");
    m_asyncCs.initialize();
    return RecluseResult_Ok;
}


ResultCode TaskManager::cleanUp()
{
    if (m_tasksMutex) 
        destroyMutex(m_tasksMutex);
        
    if (m_asyncCs.isInitialized())
        m_asyncCs.release();
        
    m_tasksMutex = nullptr;
    return RecluseResult_Ok;
}


ResultCode TaskManager::pushTask(TaskPriority priority, Task task)
{
    ScopedLock _(m_tasksMutex);
    auto it = m_tasks.find(priority);
    if (it != m_tasks.end())
    {
        it->second.push_back(task);
    }
    else
    {
        // Add new task into data structure.
        m_tasks[priority] = { task };
    }
    return RecluseResult_Ok;
}


ResultCode TaskManager::dispatchTasks(ThreadPool* pool)
{
    ScopedLock _(m_tasksMutex);
    if (!pool)
    {
        // single threaded process. would make our dispatch call syncronous.
        for (auto& priorityIt : m_tasks)
        {
            for (auto& task : priorityIt.second)
            {
                ResultCode result = task();
                R_ASSERT(result == RecluseResult_Ok);
            }
        }
    }
    else
    {
        // Multithreaded process. Can utilize multiple threads.
        // Pushes them out to the async workers, and waits until each
        // task in it's priority list is finished.
        for (auto& priorityIt : m_tasks)
        {
            std::vector<U32> ids = { };
            for (auto& task : priorityIt.second)
            {
                AsyncTaskId id = asyncTask(task, pool);
                ids.push_back(id);
            }

            for (auto id : ids)
            {
                waitForTask(id);
            }
        }
    }
    clearTasks();
    return RecluseResult_Ok;
}


void TaskManager::clearTasks()
{
    // Don't clear the whole priority structure, just the created sets.
    for (auto& taskPrioritySet : m_tasks)
    {
        taskPrioritySet.second.clear();
    }
}


static U32 processTask(void* payload)
{
    R_ASSERT(payload != nullptr);
    TaskProcess* taskProcess = static_cast<TaskProcess*>(payload);

    // Run the task process.
    while (taskProcess->isRunning())
    {
        TaskProcess::OnProcessTask onTask = taskProcess->getOnProcessTask();
        R_ASSERT(onTask != nullptr);
        ResultCode result = onTask(taskProcess);
        // Should clear all tasks regardless.
        if (result == RecluseResult_Ok)
        {
            result = taskProcess->dispatchTasks();
            R_ASSERT(result == RecluseResult_Ok);
        }
    }

    return RecluseResult_Ok;
}


ResultCode TaskProcess::start()
{
    // provide the payload.
    m_thread.payload = (void*)this;
    m_taskManager.initialize();
    m_isRunning = true;
    return createThread(&m_thread, processTask);
}


void TaskProcess::signal(Signal signal)
{ 
    switch (signal)
    {
        case Signal_Stop:
            m_isRunning = false;
            break;
        default:
            break;
    }
}


void TaskProcess::join()
{
    SizeT threadId = getCurrentThreadId();
    if ((m_isRunning == false) && threadId != m_thread.uid)
    {
        joinThread(&m_thread);
    }
}


TaskManager::AsyncTaskId TaskManager::asyncTask(Task task, ThreadPool* pool)
{
    static TaskManager::AsyncTaskId id = 0;
    static const TaskManager::AsyncTaskId InvalidId = -1; // We probably need to prevent wrap around on this value.
    TaskManager::AsyncTaskId handle = InvalidId;
    {
        ScopedCriticalSection _(m_asyncCs);
        handle = ++id;
        // Wrap around if we manage to increment to the invalid value. By this time, we shouldn't have that many 
        // tasks running on this process.
        if (handle == InvalidId)
            handle = id = 0;
        m_asyncTasks[handle] = { task, false };
    }

    // This function will be the one to run the task.
    auto TaskJobFunction = [&, handle] () -> void 
    {
        m_asyncCs.enter();
        Task asyncTask = m_asyncTasks[handle].task;
        m_asyncCs.leave();

        ResultCode code = asyncTask(); 

        m_asyncCs.enter();
        m_asyncTasks[handle].finished = true;
        m_asyncCs.leave();
    };

    // Submit the task.
    pool->submitTask(TaskJobFunction);
    return handle;
}


void TaskManager::waitForTask(TaskManager::AsyncTaskId taskId)
{
    Bool finished = false;
    // Spinlock until we finish
    while (!finished)
    {
        ScopedCriticalSection _(m_asyncCs);
        auto it = m_asyncTasks.find(taskId);
        if (it != m_asyncTasks.end())
        {
            AsyncTask task = m_asyncTasks[taskId];
            finished = task.finished;
        }
        else
        {
            // There is no task with that id, exit this blocking call.
            break;
        }
    }

    if (finished)
    {
        ScopedCriticalSection _(m_asyncCs);
        m_asyncTasks.erase(taskId);
    }
}
} // Recluse