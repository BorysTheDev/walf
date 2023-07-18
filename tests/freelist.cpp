#include <cstddef>
#include <gtest/gtest.h>
#include "freelist.h"

using namespace walf;

TEST(freelist_tests, one_thread_test) {
    constexpr auto size = 10;
    constexpr auto iterations = size - 1;
    std::int64_t arr[size];
    freelist<std::int32_t> fl(arr);
    std::int32_t* arr32[size];
    for (int i = 0; i < iterations; ++i) {
        *(arr32[i] = fl.allocate()) = i;
    }
    for (int i = 0; i < iterations; ++i) {
        EXPECT_EQ(*arr32[i], i);
        EXPECT_EQ(*std::launder(reinterpret_cast<std::int32_t*>(&arr[i])), i);
    }
    for (int i = 0; i < iterations; ++i) {
        fl.deallocate(arr32[i]);
    }
    EXPECT_EQ(std::launder(reinterpret_cast<std::int64_t*>(arr[0])), nullptr);
    for (int i = 1; i < iterations; ++i) {
        EXPECT_EQ(std::launder(reinterpret_cast<std::int64_t*>(arr[i])), &arr[i - 1]);
    }
    for (int i = 0; i < iterations; ++i) {
        *(arr32[i] = fl.allocate()) = i;
    }
    *(arr32[iterations] = fl.allocate()) = iterations;
    for (int i = 0; i < iterations; ++i) {
        EXPECT_EQ(*arr32[i], i);
        EXPECT_EQ(*std::launder(reinterpret_cast<std::int32_t*>(&arr[i])), iterations - i - 1);
    }
    EXPECT_EQ(*arr32[iterations], iterations);
    EXPECT_EQ(*std::launder(reinterpret_cast<std::int32_t*>(&arr[iterations])), iterations);
}

TEST(freelist_tests, multi_thread_test) {
    constexpr auto size = 100;
    constexpr auto thread_num = 10;
    constexpr auto iterations = size / thread_num;

    std::vector<int64_t> arr(size);
    freelist<std::int32_t> fl(arr.data());

    std::vector<std::thread> threads;
    for (int i = 0; i < thread_num; ++i) {
        threads.emplace_back([&fl] (int startPos) {
            for (int i = 0; i < iterations; ++i) {
                *fl.allocate() = startPos + i;
            }
        }, i * iterations);
    }

    for (auto& t : threads) {
        t.join();
    }

    std::vector<std::int32_t> numbers;
    for (int i = 0; i < size; ++i) {
        numbers.push_back(*std::launder(reinterpret_cast<std::int32_t*>(&arr[i])));
    }

    std::sort(numbers.begin(), numbers.end());
    for (int i = 0; i < size; ++i) {
        EXPECT_EQ(numbers[i], i);
    }

    threads.clear();
    for (int i = 0; i < thread_num; ++i) {
        threads.emplace_back([&fl, &arr] (int startPos) {
            for (int i = 0; i < iterations; ++i) {
                fl.deallocate(std::launder(reinterpret_cast<std::int32_t*>(&arr[i + startPos])));
            }
        }, i * iterations);
    }

    for (auto& t : threads) {
        t.join();
    }

    // check memory for correct free list
}
