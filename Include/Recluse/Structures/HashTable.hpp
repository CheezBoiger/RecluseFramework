//
#pragma once

#include "Recluse/Types.hpp"

namespace Recluse {


template<typename _Key, typename _Value, typename _Hash, typename _Comparer, typename _CompareEqual, typename _Allocator>
class hash_table
{
private:

    struct KeyValuePair
    {
        _Key    _key;
        _Value  _value;
    };

public:

    hash_table() { }

private:
    _Comparer       _comparer;
    _CompareEqual   _equal;
    _Allocator      _allocator;
    _Hash           _hasher;
    KeyValuePair*   _table;
    U32             _entriesCount;
    U32             _totalEntriesCount;
};

// Fixed unordered map is a cache friendly map structure that is intended to stay with 
// a constant sized table, and utilize open addressing to store collisions. It is best to use
// for small data storage and caching, not intended for scaling.
template<typename _key, typename _value, size_t fixed_size, typename _hash = std::hash<_key>>
class fixed_unordered_map
{
public:
    typedef _value& value_reference;
    typedef const _value& const_value_reference;
    typedef _value* value_pointer;
    typedef const _value* const_value_pointer;
    typedef _key& key_referene;

    struct _key_value_pair
    {
        _key key;
        _value value;
        Bool isUsed = false;
        _key_value_pair() : isUsed(false) { }
    };

    typedef fixed_unordered_map* _storage_ptr;

    struct iterator
    {
    private:
        _storage_ptr storage;
        size_t index;
        void skip_if_invalid() {
            if (!storage) return;
            while (index < storage->max_size && !storage->container[index].isUsed)
                ++index;
        }
    public:
        iterator(_storage_ptr storage, size_t index)
            : storage(storage), index(index) { skip_if_invalid(); }

        const_value_reference operator*() const { return storage->container[index].value; }
        value_reference operator*() { return storage->container[index].value; }

        value_pointer operator->() { return &storage->container[index].value; }
        const_value_pointer operator->() const { return &storage->container[index].value; }

        bool operator==(const iterator& other) const {
            return (storage == other.storage) && (index == other.index);
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

        iterator operator++(int) {
            iterator temp = *this;
            ++(*this);
            return tmp;
        }

        iterator operator++() {
            ++index;
            skip_if_invalid();
        }
    };

    typedef const iterator const_iterator;

    fixed_unordered_map(size_t new_size = fixed_size) 
        : max_size(new_size) {
        container = new _key_value_pair[max_size];    
    }

    fixed_unordered_map(const fixed_unordered_map& map) { }
    fixed_unordered_map(fixed_unordered_map&& map) { }

    ~fixed_unordered_map() {
        delete[] container;
    }

    fixed_unordered_map& operator=(const fixed_unordered_map& map) const { return *this; }
    fixed_unordered_map& operator=(fixed_unordered_map&& map) { return *this; }

    value_reference operator[](const _key& key) {
        size_t start_index = _hasher(key) % max_size;
        size_t index = start_index;
        do {
            if (!container[index].isUsed) {
                container[index].key = key;
                container[index].isUsed = true;
                return container[index].value;
            }
            if (container[index].isUsed && (container[index].key == container[index].key)) {
                return container[index].value;
            }
        } while (index != start_index);
        return container[index].value;
    }
    const _value& operator[](const _key& key) const;

    iterator find(const _key& key) {
        if (max_size == 0) return end();
        size_t start_idx = _hasher(key) % max_size;
        size_t index = start_idx;
        do {
            if (container[index].isUsed && container[index].key == key) {
                return iterator(this, index);
            }
            index = (index + 1) % max_size;
        } while (index != start_idx);
        return end();
    }

    iterator find(const _key& key) const {
        if (max_size == 0) return end();
        size_t start_idx = _hasher(key) % max_size;
        size_t index = start_idx;
        do {
            if (container[index].isUsed && container[index].key == key) {
                return iterator(this, index);
            }
            index = (index + 1) % max_size;
        } while (index != start_idx);
        return end();
    }

    const_iterator begin() { return iterator(this, 0); }
    const_iterator end() { return iterator(this, max_size); }
    
private:
    _hash _hasher;
    _key_value_pair* container;
    size_t max_size;
}; 
} // Recluse