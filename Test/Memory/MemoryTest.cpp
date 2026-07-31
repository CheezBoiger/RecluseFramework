//
#include <gtest/gtest.h>
#include <Recluse/Memory/LinearScratchMemory.hpp>

TEST(MemoryTest, LinearScratchMemoryTest)
{
    using namespace Recluse;
    LinearScratchMemory<1024> scratchMemory;
    const int varTest = 42;

    const float floatTest = 5.234f;
    struct TestStruct { int v; char f; } testStruct;

    struct TestStruct* mo = scratchMemory.allocate<struct TestStruct>();
    int* t0 = scratchMemory.allocate<int>();

    *t0 = varTest;

    float* t1 = scratchMemory.allocate<float>();

    *t1 = floatTest;

    struct TestStruct* t2 = scratchMemory.allocate<struct TestStruct>(2);

    EXPECT_EQ(varTest, *t0);
    EXPECT_EQ(floatTest, *t1);
} 