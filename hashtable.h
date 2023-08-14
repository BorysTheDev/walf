#ifndef HASHTABLE_H
#define HASHTABLE_H
#include <cstddef>
#include <utility>
#include <functional>
#include <memory>

template <class Key, class Value, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>>
class hashtable {
public:
    using value_type = std::pair<Key, Value>;

    hashtable(void* ptr, size_t mem_size) : _mem_size(mem_size) {
        _buckets = new (ptr) bucket[_buckets_num];
    }

    bool emplace(Key key, Value val) {
        const auto h = Hash()(key);
        const auto bucket_pos = h % _mem_size;
        auto& buck = _buckets[bucket_pos];
        const auto* el = std::make_unique<value_type>(std::move(key), std::move(val));
        const auto hint_pos = bucket_pos % bucket::size;
        for (auto i = hint_pos; i < bucket::size; ++i) {
            if (buck.elements[i] == nullptr) {
                buck.elements[i] = el;
                return true;
            }
        }
        for (auto i = 0; i < hint_pos; ++i) {
            if (buck.elements[i] == nullptr) {
                buck.elements[i] = el;
                return true;
            }
        }
        return extend(h, el);
    }

    bool extend(std::uint64_t hash, value_type* val) {

    }

private:
    struct bucket {
        static constexpr auto mem_size = 128;

        static constexpr auto size = mem_size / sizeof(value_type*);
        value_type* elements[size] = {};

        static_assert(sizeof(bucket) == mem_size);
    };

    size_t _mem_size;
    size_t _buckets_num = 4096 / bucket::mem_size;
    bucket* _buckets;
};

#endif // HASHTABLE_H
