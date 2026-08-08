//
#include <Recluse/Memory/LinearAllocationStrategy.hpp>
#pragma once

namespace Recluse {


template<typename Type, typename _Allocator = LinearAllocator>
class Array
{
public:
    Array(size_t count)
        : count(0)
        , data(nullptr)
        , maxCount(count) 
    {
        if (maxCount != 0)
            data = (Type*)allocator.allocate(sizeof(Type) * maxCount);
    }

    ~Array()
    {
        if (count)
            allocator.free(data);
        allocator.cleanUp();
        data = nullptr;
        maxCount = 0;
        count = 0;
    }

    Type& operator[](size_t index) { return data[index]; }
    const Type& operator[](size_t index) const { return data[index]; }

    void resize(size_t count) { }
    void reserve(size_t count) { }
private:
    _Allocator allocator;
    Type* data;
    size_t count;
    size_t maxCount;
};
} // Recluse