//
#include <Recluse/Threading/Threading.hpp>
#include <Recluse/Threading/ThreadPool.hpp>
#include <Recluse/Structures/HashTable.hpp>

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