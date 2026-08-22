//
#include <Recluse/Threading/Threading.hpp>
#include <Recluse/Threading/ThreadPool.hpp>

#include <array>
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
    ThreadPool pool(2);

    pool.start();

    std::array<int, 10> unsorted = { 2, 4, 1, 0, 3, 4, 1, 0, 3, 2 };
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

    pool.waitIdle();

    for (uint i = 0; i < unsorted.size(); ++i)
    {
        EXPECT_EQ(i % 5, unsorted[i]);
    }
}