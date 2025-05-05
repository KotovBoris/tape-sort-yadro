#include "external_sort.hpp"

#include "vector_tape.hpp"
#include "helpers.hpp"

#include <vector>
#include <algorithm>
#include <random>

#include <gtest/gtest.h>


TEST(ChunkMergeSortTest, UniqueElementsDefault) {
    std::vector<int32_t> input = {3, 2, 1};
    VectorTape in_t(input);
    VectorTape out_t(std::vector<int32_t>(input.size(), 0));

    ext_sort::ChunkMergeSort(in_t, out_t, 8, false);
    EXPECT_EQ(TapeToVector(out_t), (std::vector<int32_t>{1,2,3}));
}

TEST(ChunkMergeSortTest, UniqueElementsHeap) {
    std::vector<int32_t> input = {10, 7, 8, 9, 6};
    VectorTape in_t(input);
    VectorTape out_t(std::vector<int32_t>(input.size(), 0));

    ext_sort::ChunkMergeSort(in_t, out_t, 1024, true);
    EXPECT_EQ(TapeToVector(out_t), (std::vector<int32_t>{6,7,8,9,10}));
}

TEST(ChunkMergeSortTest, DuplicatesAndNegatives) {
    std::vector<int32_t> input = {0, -1, 2, -1, 0, 2, 1};
    VectorTape in_t(input);
    VectorTape out_t(std::vector<int32_t>(input.size(), 0));

    ext_sort::ChunkMergeSort(in_t, out_t, 512, false);
    auto result = TapeToVector(out_t);
    std::vector<int32_t> expected = {-1,-1,0,0,1,2,2};
    EXPECT_EQ(result, expected);
}

TEST(ChunkMergeSortTest, EmptyInput) {
    std::vector<int32_t> input;
    VectorTape in_t(input);
    VectorTape out_t(input);

    ext_sort::ChunkMergeSort(in_t, out_t, 256, false);
    EXPECT_TRUE(TapeToVector(out_t).empty());
}

TEST(ChunkMergeSortTest, RandomLarge) {
    std::vector<int32_t> input = RandomVector(10000, -1000, 1000);

    std::vector<int32_t> expected = input;
    std::sort(expected.begin(), expected.end());

    for (bool heap : {false, true}) {
        for (size_t memory_limit : {8, 64, 512, 2048, 8192}) {
            VectorTape in_t(input);
            VectorTape out_t(std::vector<int32_t>(10000, 0));
            ext_sort::ChunkMergeSort(in_t, out_t, 8192, heap);
            EXPECT_EQ(TapeToVector(out_t), expected);
        }
    }
}

// Недостаточно памяти (меньше sizeof(int32_t))
TEST(ChunkMergeSortTest, MemoryTooSmall) {
    std::vector<int32_t> input = {1,2,3};
    VectorTape in_t(input);
    VectorTape out_t(std::vector<int32_t>(input.size(), 0));
    EXPECT_THROW(ext_sort::ChunkMergeSort(in_t, out_t, /*memory_limit_bytes=*/3, false), std::runtime_error);
}