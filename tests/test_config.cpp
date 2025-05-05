#include "config.hpp"
#include "delays.hpp"

#include "helpers.hpp"

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>


TEST(ConfigTest, ValidFullConfig) {
    const std::string fname = "test_full_config.yaml";
    std::string yaml = R"(
        delays:
          read_ms: 10
          write_ms: 20
          shift_ms: 30
          rewind_ms: 40
        memory_limit_bytes: 12345
        strict_stack_limit: true
        value_range: [ -5, 15 ]
    )";
    WriteYaml(fname, yaml);

    Config cfg = Config::Load(fname);
    EXPECT_EQ(cfg.delays.read_ms, 10u);
    EXPECT_EQ(cfg.delays.write_ms, 20u);
    EXPECT_EQ(cfg.delays.shift_ms, 30u);
    EXPECT_EQ(cfg.delays.rewind_ms, 40u);
    EXPECT_EQ(cfg.memory_limit_bytes, 12345u);
    EXPECT_TRUE(cfg.strict_stack_limit);
    ASSERT_TRUE(cfg.value_min.has_value());
    ASSERT_TRUE(cfg.value_max.has_value());
    EXPECT_EQ(cfg.value_min.value(), -5);
    EXPECT_EQ(cfg.value_max.value(), 15);
}

TEST(ConfigTest, ValidNoRange) {
    const std::string fname = "test_no_range.yaml";
    std::string yaml = R"(
        delays:
          read_ms: 1
          write_ms: 2
          shift_ms: 3
          rewind_ms: 4
        memory_limit_bytes: 1000
        strict_stack_limit: false
    )";
    WriteYaml(fname, yaml);

    Config cfg = Config::Load(fname);
    EXPECT_EQ(cfg.delays.read_ms, 1u);
    EXPECT_EQ(cfg.delays.write_ms, 2u);
    EXPECT_EQ(cfg.delays.shift_ms, 3u);
    EXPECT_EQ(cfg.delays.rewind_ms, 4u);
    EXPECT_EQ(cfg.memory_limit_bytes, 1000u);
    EXPECT_FALSE(cfg.strict_stack_limit);
    EXPECT_FALSE(cfg.value_min.has_value());
    EXPECT_FALSE(cfg.value_max.has_value());
}

TEST(ConfigTest, InvalidMissingField) {
    const std::string fname = "test_missing_field.yaml";
    std::string yaml = R"(
        delays:
          write_ms: 20
          shift_ms: 30
          rewind_ms: 40
        memory_limit_bytes: 123
        strict_stack_limit: false
    )"; // read_ms missing
    WriteYaml(fname, yaml);
    EXPECT_THROW(Config::Load(fname), std::exception);
}

TEST(ConfigTest, InvalidValueRangeSize) {
    const std::string fname = "test_bad_range.yaml";
    std::string yaml = R"(
        delays:
          read_ms: 5
          write_ms: 5
          shift_ms: 5
          rewind_ms: 5
        memory_limit_bytes: 500
        strict_stack_limit: false
        value_range: [1]
    )"; // wrong size
    WriteYaml(fname, yaml);
    Config cfg = Config::Load(fname);
    EXPECT_FALSE(cfg.value_min.has_value());
    EXPECT_FALSE(cfg.value_max.has_value());
}

TEST(ConfigTest, NonexistentFile) {
    EXPECT_THROW(Config::Load("nonexistent.yaml"), std::exception);
}
