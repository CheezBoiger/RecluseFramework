//
#pragma once

#include "Recluse/Types.hpp"

namespace Recluse {


template<typename _Key, typename _Value, typename _Hash, typename _Comparer, typename _CompareEqual, typename _Allocator>
class HashTable
{
private:

    struct KeyValuePair
    {
        _Key    _key;
        _Value  _value;
    };

public:

    HashTable();

private:
    _Comparer       _comparer;
    _CompareEqual   _equal;
    _Allocator      _allocator;
    _Hash           _hasher;
    KeyValuePair*   _table;
    U32             _entriesCount;
    U32             _totalEntriesCount;
};


template<typename _key, typename _value, size_t fixed_size, bool dynamic, typename _hash = std::hash<_key>>
class fixed_unordered_map
{
public:
    struct iterator
    {
        _key first;
        _value& second;
    };

    fixed_unordered_map(size_t new_size = fixed_size);
    fixed_unordered_map(const fixed_unordered_map& map);
    fixed_unordered_map(fixed_unordered_map&& map);

    fixed_unordered_map& operator=(const fixed_unordered_map& map) const;
    fixed_unordered_map& operatpr=(fixed_unordered_map&& map)

    _value& operator[](const _key& key);
    const _value& operator[](const _key& key) const;

    iterator& find(const _key& key);
    const iterator& find(const _key& key) const;

private:
    struct _key_value_pair
    {
        _key key;
        _value value;
    };

    _hash _hasher;
    _key_value_pair* container;
    size_t max_size;
}; 
} // Recluse