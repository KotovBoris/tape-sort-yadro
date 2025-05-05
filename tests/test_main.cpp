#include "config.hpp"
#include "delays.hpp"
#include "file_sort.hpp"

#include "helpers.hpp"

#include <cstdint>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <vector>

#include <gtest/gtest.h>


TEST(FileSortTest, CountingSortRandom) {
    const std::string input = "test_fs_count_in.bin";
    const std::string output = "test_fs_count_out.bin";
    const std::string cfg = "test_fs_count.yaml";

    auto data = RandomVector(1000, -100, 100);
    WriteIntFile(input, data);

    std::string yaml = R"(
        delays:
          read_ms: 0
          write_ms: 0
          shift_ms: 0
          rewind_ms: 0
        memory_limit_bytes: 40
        strict_stack_limit: false
        value_range: [ -100, 100 ]
    )";
    WriteYaml(cfg, yaml);

    ext_sort::FileSort(input, output, cfg);

    auto sorted = ReadIntFile(output);
    std::vector<int32_t> expected = data;
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(sorted, expected);
}

TEST(FileSortTest, ChunkMergeSortRandom) {
    const std::string input = "test_fs_chunk_in.bin";
    const std::string output = "test_fs_chunk_out.bin";
    const std::string cfg = "test_fs_chunk.yaml";

    auto data = RandomVector(1000, -5000, 5000);
    WriteIntFile(input, data);

    std::string yaml = R"(
        delays:
          read_ms: 0
          write_ms: 0
          shift_ms: 0
          rewind_ms: 0
        memory_limit_bytes: 40
        strict_stack_limit: )";

    for (auto strict_stack_limit : {"true", "false"}) {
      WriteYaml(cfg, yaml + strict_stack_limit);

      ext_sort::FileSort(input, output, cfg);
      auto sorted = ReadIntFile(output);
      std::vector<int32_t> expected = data;
      std::sort(expected.begin(), expected.end());
      EXPECT_EQ(sorted, expected);
    }
}

TEST(FileSortTest, MissingInputFile) {
    const std::string input = "nonexistent_in.bin";
    const std::string output = "should_not_create.bin";
    const std::string cfg = "test_fs_missing_cfg.yaml";
    std::string yaml = R"(
        delays:
          read_ms: 0
          write_ms: 0
          shift_ms: 0
          rewind_ms: 0
        memory_limit_bytes: 512
        strict_stack_limit: false
    )";
    WriteYaml(cfg, yaml);

    EXPECT_THROW(ext_sort::FileSort(input, output, cfg), std::exception);
}
