//
#include <gtest/gtest.h>
#include <Recluse/Memory/LinearScratchMemory.hpp>

TEST(MemoryTest, LinearScratchMemoryStaticAllocate)
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

TEST(MemoryTest, LinearScratchMemoryDynamic)
{
    using namespace Recluse;
    // Simple scratch memory, re-sizes to 32 byte allocations.
    LinearScratchMemory<8, true> alloc;

    U64* c0 = alloc.allocate<U64>();
    ASSERT_NE(c0, nullptr);
    *c0 = 42;
    U64* c1 = alloc.allocate<U64>();
    ASSERT_NE(c1, nullptr);
    *c1 = 35;
    U64* c2 = alloc.allocate<U64>();
    ASSERT_NE(c2, nullptr);
    *c2 = 900;
    U64* c3 = alloc.allocate<U64>();
    ASSERT_NE(c3, nullptr);
    *c3 = 1030;

    U64* address = (U64*)alloc.getBaseAddress();

    EXPECT_EQ(42, address[0]);
    EXPECT_EQ(35, address[1]);
    EXPECT_EQ(900, address[2]);
    EXPECT_EQ(1030, address[3]);
}

TEST(MemoryTest, LinearScratchMemoryDynamicHuge)
{
    using namespace Recluse;
    LinearScratchMemory<8, true> alloc;

    for (u32 i = 0; i < 1000; ++i)
    {
        U64* ptr = alloc.allocate<U64>();
        ASSERT_NE(ptr, nullptr);
        *ptr = i;
    }

    U64* array = (U64*)alloc.getBaseAddress();
    for (u32 i = 0; i < 1000; ++i)
    {
        EXPECT_EQ(array[i], i);
    }
}

TEST(MemoryTest, LinearScratchMemoryDynamicPaged)
{
    using namespace Recluse;
    // Simple scratch memory, re-sizes to 32 byte allocations. Paged to 4096
    LinearScratchMemory<8, true, 4096ull> alloc;

    U64* c0 = alloc.allocate<U64>();
    *c0 = 42;
    U64* c1 = alloc.allocate<U64>();
    *c1 = 35;
    U64* c2 = alloc.allocate<U64>();
    *c2 = 900;
    U64* c3 = alloc.allocate<U64>();
    *c3 = 1030;

    U64* address = (U64*)alloc.getBaseAddress();

    EXPECT_EQ(42, address[0]);
    EXPECT_EQ(35, address[1]);
    EXPECT_EQ(900, address[2]);
    EXPECT_EQ(1030, address[3]);
}

TEST(MemoryTest, MemoryPoolClearDefault)
{
    using namespace Recluse;
    char data[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    {
        MemoryArena arena((void*)data, 10 * sizeof(char));
        arena.clear(0xF);
    }

    EXPECT_EQ(data[0], 0xF);
    EXPECT_EQ(data[1], 0xF);

    {
        MemoryArena arena((void*)data, 10 * sizeof(char));
        char* ptr = (char*)arena.getBaseAddress();
        *ptr = 1;
    }

    EXPECT_EQ(data[0], 1);
    EXPECT_EQ(data[9], 0xF);
}