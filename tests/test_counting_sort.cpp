#include "external_sort.hpp"
#include "tape.hpp"

#include "helpers.hpp"
#include "vector_tape.hpp"

#include <cstdint>

#include <algorithm>
#include <numeric>
#include <random>
#include <vector>

#include <gtest/gtest.h>


TEST(CountingSortTest, UniqueElements) {
    std::vector<int32_t> input = {5,1,3,2,4};
    VectorTape in_t(input), out_t(std::vector<int32_t>(input.size(),0));

    ext_sort::CountingSort(in_t, out_t, 1024);
    EXPECT_EQ(TapeToVector(out_t), (std::vector<int32_t>{1,2,3,4,5}));
}

TEST(CountingSortTest, WithDuplicatesAndNegatives) {
    std::vector<int32_t> input = {0,-1,2,-1,0,2,1};
    VectorTape in_t(input), out_t(std::vector<int32_t>(input.size(),0));

    ext_sort::CountingSort(in_t, out_t, 2048);
    EXPECT_EQ(TapeToVector(out_t), (std::vector<int32_t>{-1,-1,0,0,1,2,2}));
}

TEST(CountingSortTest, AllEqualElements) {
    std::vector<int32_t> input(10,42);
    VectorTape in_t(input), out_t(std::vector<int32_t>(input.size(),0));

    ext_sort::CountingSort(in_t, out_t, 512);
    EXPECT_EQ(TapeToVector(out_t), std::vector<int32_t>(10,42));
}

TEST(CountingSortTest, ExplicitWindowing) {
    std::vector<int32_t> input = {9,8,7,6,5,4,3,2,1,0};
    VectorTape in_t(input), out_t(std::vector<int32_t>(input.size(),0));

    ext_sort::CountingSort(in_t, out_t, 64, 0, 9);
    EXPECT_EQ(TapeToVector(out_t), (std::vector<int32_t>{0,1,2,3,4,5,6,7,8,9}));
}

TEST(CountingSortTest, LargeReversedWindowing) {
    std::vector<int32_t> input(100);
    for (int i = 0; i < 100; ++i) {
        input[i] = 99 - i;
    }
    VectorTape in_t(input), out_t(std::vector<int32_t>(input.size(),0));

    ext_sort::CountingSort(in_t, out_t, 80);
    auto result = TapeToVector(out_t);
    std::vector<int32_t> expected(100);
    std::iota(expected.begin(), expected.end(), 0);
    EXPECT_EQ(result, expected);
}

TEST(CountingSortTest, MemoryTooSmallAuto) {
    std::vector<int32_t> input = {1,2,3};
    VectorTape in_t(input), out_t(std::vector<int32_t>(input.size(),0));

    EXPECT_THROW(ext_sort::CountingSort(in_t, out_t, 12), std::runtime_error);
}

TEST(CountingSortTest, MemoryTooSmallExplicit) {
    std::vector<int32_t> input = {1,2,3};
    VectorTape in_t(input), out_t(std::vector<int32_t>(input.size(),0));

    EXPECT_THROW(ext_sort::CountingSort(in_t, out_t, 12, 1, 3), std::runtime_error);
}

TEST(CountingSortTest, RandomLargeAuto) {
    auto input = RandomVector(500, -1000, 1000);

    std::vector<int32_t> expected = input;
    std::sort(expected.begin(), expected.end());

    VectorTape in_t(input), out_t(std::vector<int32_t>(500,0));
    ext_sort::CountingSort(in_t, out_t, 32);
    EXPECT_EQ(TapeToVector(out_t), expected);
}

TEST(CountingSortTest, RandomLargeExplicit) {
    auto input = RandomVector(500, 0, 500);

    std::vector<int32_t> expected = input;
    std::sort(expected.begin(), expected.end());

    VectorTape in_t(input), out_t(std::vector<int32_t>(500,0));
    ext_sort::CountingSort(in_t, out_t, 32, 0, 500);
    EXPECT_EQ(TapeToVector(out_t), expected);
}
