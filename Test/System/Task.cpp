//
#include <Recluse/Threading/Threading.hpp>
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