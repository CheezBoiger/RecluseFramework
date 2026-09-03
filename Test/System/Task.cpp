//
#include <Recluse/Threading/Threading.hpp>
#include <Recluse/Threading/ThreadPool.hpp>
#include <Recluse/Structures/HashTable.hpp>

#include <array>
#include <atomic>
#include <chrono>
#include <numeric>
#include <gtest/gtest.h>

using namespace Recluse;


struct Param
{
    uint* dataOut;
    uint index;
};

static ResultCode threadRun(void* args)
{
    Param& params = *static_cast<Param*>(args);
    params.dataOut[params.index] = params.index;
    return RecluseResult_Ok;
}

TEST(Task, ThreadsSimple)
{
    Thread threads[4];

    uint databuf[4] = { 0xf, 0xf, 0xf, 0xf };
    
    Param params[4];
    for (uint i = 0; i < 4; ++i)
    {
        params[i].dataOut = databuf;
        params[i].index = i;
        threads[i].payload = &params[i]; 
        createThread(&threads[i], threadRun); 
    }

    for (uint i = 0; i < 4; ++i)
        joinThread(&threads[i]);

    for (uint i = 0; i < 4; ++i)
    {
        EXPECT_NE(0xf, databuf[i]);

        EXPECT_EQ(i, databuf[i]);
    }
}

struct Param1
{
    uint* val;
    Mutex mut;
    U64 mainThreadId;
};

static ResultCode countThreadFun(void* args)
{
    Param1* param = reinterpret_cast<Param1*>(args);
    EXPECT_NE(getCurrentThreadId(), param->mainThreadId);
    if (lockMutex(param->mut) == RecluseResult_Ok)
    {
        ++(*param->val);
        unlockMutex(param->mut);
    }
    return RecluseResult_Ok;
}

TEST(Task, MutexCounter)
{
    Thread threads[4];
    Mutex mutex = createMutex("SimpleMutexDoesNotNeedName");
    uint result = 0;

    Param1 param;
    param.mut = mutex;
    param.val = &result;
    param.mainThreadId = getCurrentThreadId();

    for (uint i = 0; i < 4; ++i)
    {
        threads[i].payload = &param; 
        createThread(&threads[i], countThreadFun); 
    }

    for (uint i = 0; i < 4; ++i)
        joinThread(&threads[i]);

    destroyMutex(mutex);

    EXPECT_EQ(result, 4);
}

TEST(Task, ThreadPoolTest)
{
    ThreadPool otherPool(0);
    std::array<int, 10> unsorted = { 2, 4, 1, 0, 3, 4, 1, 0, 3, 2 };

    {
        ThreadPool pool(2);

        pool.start();
        pool.submitTask([&] () -> void {
            uint start = 0;
            uint end = 5;
            std::sort(unsorted.begin() + start, unsorted.begin() + end);
        });

        pool.submitTask([&] () -> void {
            uint start = 5;
            uint end = 10;
            std::sort(unsorted.begin() + start, unsorted.begin() + end);
        });

        otherPool = std::move(pool);
    }

    otherPool.waitIdle();

    for (uint i = 0; i < unsorted.size(); ++i)
    {
        EXPECT_EQ(i % 5, unsorted[i]);
    }
}

TEST(StructureTest, TestFixedMap)
{
    fixed_unordered_map<int, char, 8> stuff;
    stuff[1] = 'c';
    stuff[2] = 'd';
    stuff[1] = 's';

    auto& it = stuff.find(1);
    EXPECT_NE(it, stuff.end());
    EXPECT_EQ(*it, 's');
    
    auto& ff = stuff.find(2);
    EXPECT_NE(ff, stuff.end());
    EXPECT_EQ(*ff, 'd');

    auto& mm = stuff.find(128);
    EXPECT_EQ(mm, stuff.end());
}

TEST(StructureTest, FixedMapBig)
{
    fixed_unordered_map<int, std::string, 500> little;
    
    for (uint i = 0; i < 100; ++i)
    {
        little[i] = std::to_string(i);
    }

    for (uint i = 0; i < 100; ++i)
    {
        auto it = little.find(i);
        EXPECT_NE(it, little.end());
        EXPECT_EQ(*it, std::to_string(i));
    }
}

TEST(Task, ValidatesWorkerIds) 
{
    const uint32_t numWorkers = 4;
    Recluse::ThreadPool pool(numWorkers);

    // 1. Verify worker count before starting
    EXPECT_EQ(pool.getWorkerCount(), numWorkers);

    pool.start();

    // 2. Collect all worker IDs exposed by the pool
    std::vector<Recluse::U64> knownWorkerIds;
    knownWorkerIds.reserve(numWorkers);

    for (uint32_t i = 0; i < pool.getWorkerCount(); ++i) 
    {
        Recluse::U64 id = pool.getWorkerId(i);
        // Ensure ID is set and not equal to the default uninitialized index constant (~0)
        EXPECT_NE(id, static_cast<Recluse::U64>(Recluse::ThreadPool::kBadIndex));
        knownWorkerIds.push_back(id);
    }

    // 3. Submit tasks that record the execution thread ID
    std::atomic<int> completedTasks{0};
    std::vector<Recluse::U64> executedOnThreads(numWorkers);

    for (uint32_t i = 0; i < numWorkers; ++i) {
        pool.submitTask([i, &executedOnThreads, &completedTasks]() {
            // Get current thread ID (Assuming Threading infrastructure provides a way, 
            // or cast std::this_thread::get_id() hash / integer equivalent)
            Recluse::U64 currentThreadId = getCurrentThreadId(); // static_cast<Recluse::U64>(
            //    std::hash<std::thread::id>{}(std::this_thread::get_id())
            //);

            executedOnThreads[i] = currentThreadId;
            completedTasks.fetch_add(1, std::memory_order_relaxed);
        });
    }

    pool.waitIdle();
    EXPECT_EQ(completedTasks.load(), numWorkers);

    // 4. Verify that each execution thread ID matches one of the known worker IDs
    for (Recluse::U64 execId : executedOnThreads) {
        auto it = std::find(knownWorkerIds.begin(), knownWorkerIds.end(), execId);
        EXPECT_NE(it, knownWorkerIds.end()) 
            << "Task executed on thread ID " << execId 
            << ", which does not match any registered worker ID in ThreadPool.";
    }

    pool.stop();
}

class ThreadPoolTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Shared test fixture setup if needed
    }
};

// 1. Basic Task Execution
TEST_F(ThreadPoolTest, SubmitsAndExecutesBasicTask) {
    ThreadPool pool(2);
    pool.start();

    std::atomic<bool> executed{false};
    pool.submitTask([&executed]() {
        executed = true;
    });

    pool.waitIdle();
    EXPECT_TRUE(executed.load());
    pool.stop();
}

// 2. Variadic Argument Forwarding
TEST_F(ThreadPoolTest, ForwardsMultipleArgumentsCorrectly) {
    ThreadPool pool(2);
    pool.start();

    int resultInt = 0;
    std::string resultStr;

    auto task = [](int a, double b, const std::string& str, int& outInt, std::string& outStr) {
        outInt = a + static_cast<int>(b);
        outStr = str + " World";
    };

    pool.submitTask(task, 10, 5.5, "Hello", std::ref(resultInt), std::ref(resultStr));
    pool.waitIdle();

    EXPECT_EQ(resultInt, 15);
    EXPECT_EQ(resultStr, "Hello World");
    pool.stop();
}

// 3. Move-Only Types Forwarding
//TEST_F(ThreadPoolTest, HandlesMoveOnlyArguments) {
//    ThreadPool pool(2);
//    pool.start();
//
//    auto uniquePtr = std::make_unique<int>(42);
//    std::atomic<int> value{0};
//
//    auto task = [](std::unique_ptr<int> ptr, std::atomic<int>& val) {
//        val = *ptr;
//    };
//
//    pool.submitTask(task, std::move(uniquePtr), std::ref(value));
//    pool.waitIdle();
//
//    EXPECT_EQ(value.load(), 42);
//    pool.stop();
//}

// 4. Concurrent Throughput & Data Races
TEST_F(ThreadPoolTest, ExecutesMultipleTasksConcurrently) {
    const uint32_t numThreads = 4;
    const int taskCount = 1000;
    ThreadPool pool(numThreads);
    pool.start();

    std::atomic<int> counter{0};
    for (int i = 0; i < taskCount; ++i) {
        pool.submitTask([&counter]() {
            counter.fetch_add(1, std::memory_order_relaxed);
        });
    }

    pool.waitIdle();
    EXPECT_EQ(counter.load(), taskCount);
    pool.stop();
}

// 5. Order Independence / Parallel Execution Proof
TEST_F(ThreadPoolTest, WorkersExecuteInParallel) {
    ThreadPool pool(2);
    pool.start();

    std::atomic<int> activeWorkers{0};
    std::atomic<bool> overlapDetected{false};

    auto longTask = [&activeWorkers, &overlapDetected]() {
        int current = ++activeWorkers;
        if (current > 1) {
            overlapDetected = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        --activeWorkers;
    };

    pool.submitTask(longTask);
    pool.submitTask(longTask);

    pool.waitIdle();
    EXPECT_TRUE(overlapDetected.load());
    pool.stop();
}

// 6. Member Function Binding
struct SampleWork {
    int value = 0;
    void add(int amount) { value += amount; }
};

TEST_F(ThreadPoolTest, BindsMemberFunctions) {
    ThreadPool pool(1);
    pool.start();

    SampleWork obj;
    pool.submitTask(&SampleWork::add, &obj, 25);

    pool.waitIdle();
    EXPECT_EQ(obj.value, 25);
    pool.stop();
}

// 7. Stop Drains Remaining Tasks
TEST_F(ThreadPoolTest, StopFlushesEnqueuedTasks) {
    ThreadPool pool(1);
    pool.start();

    std::atomic<int> completedTasks{0};

    // Block the single worker briefly
    pool.submitTask([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    });

    // Queue up additional tasks
    for (int i = 0; i < 5; ++i) {
        pool.submitTask([&completedTasks]() {
            completedTasks.fetch_add(1);
        });
    }

    // stop() should block until all enqueued tasks complete
    pool.waitIdle();
    pool.stop();
    EXPECT_EQ(completedTasks.load(), 5);
}