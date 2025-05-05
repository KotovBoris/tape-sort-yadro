#include "delays.hpp"
#include "file_tape.hpp"

#include "helpers.hpp"

#include <cstddef>
#include <cstdlib>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <new>
#include <thread>

#include <gtest/gtest.h>


using namespace std::chrono_literals;


// Глобальные счётчики аллокаций
static size_t current_alloc = 0;
static size_t max_alloc = 0;

// Отслеживание использования памяти
void* operator new(std::size_t sz) {
    current_alloc += sz;
    max_alloc = std::max(max_alloc, current_alloc);

    void* p = std::malloc(sz);

    if (!p) {
      throw std::bad_alloc();
    }

    return p;
}

void operator delete(void* p, std::size_t sz) noexcept {
    current_alloc -= sz;
    std::free(p);
}

void operator delete(void* p) noexcept {
    std::free(p);
}

// Тест чтения, записи и позиций с проверкой задержек
TEST(FileTapeTest, ReadWriteAndPosition) {
    const std::string fname = "test_tape.bin";
    std::vector<int32_t> initial = {10, 20, 30};
    WriteIntFile(fname, initial);

    FileTape tape(fname, Delays{50, 60, 70, 80});
    EXPECT_EQ(tape.Size(), initial.size());
    EXPECT_EQ(tape.Position(), 0u);

    auto t0 = std::chrono::steady_clock::now();
    int32_t v0 = tape.Read();
    auto dt = std::chrono::steady_clock::now() - t0;
    EXPECT_GE(v0, initial[0]);
    EXPECT_EQ(tape.Position(), 0u);
    EXPECT_GE(dt, 50ms);

    t0 = std::chrono::steady_clock::now();
    tape.Next();
    dt = std::chrono::steady_clock::now() - t0;
    EXPECT_GE(dt, 70ms);
    EXPECT_EQ(tape.Position(), 1u);
    EXPECT_EQ(tape.Read(), initial[1]);

    t0 = std::chrono::steady_clock::now();
    tape.Prev();
    dt = std::chrono::steady_clock::now() - t0;
    EXPECT_GE(dt, 70ms);
    EXPECT_EQ(tape.Position(), 0u);
}

TEST(FileTapeTest, RewindAndWrite) {
    const std::string fname = "test_tape2.bin";
    std::vector<int32_t> initial = {1,2,3,4};
    WriteIntFile(fname, initial);

    FileTape tape(fname, Delays{10, 20, 30, 40});
    tape.Next(); tape.Next(); tape.Next();
    EXPECT_EQ(tape.Position(), 3u);

    auto t0 = std::chrono::steady_clock::now();
    tape.Rewind(-2);
    auto dt = std::chrono::steady_clock::now() - t0;
    EXPECT_GE(dt, 40ms);
    EXPECT_EQ(tape.Position(), 1u);

    t0 = std::chrono::steady_clock::now();
    tape.Write(99);
    dt = std::chrono::steady_clock::now() - t0;
    EXPECT_GE(dt, 20ms);
    EXPECT_EQ(tape.Read(), 99);
}

TEST(FileTapeTest, CreateTemporaryAndMemoryLimit) {
    const std::string fname = "test_tape3.bin";
    std::vector<int32_t> initial = {7,8,9};
    WriteIntFile(fname, initial);

    FileTape tape(fname, Delays{0,0,0,0});
    auto tmp = tape.CreateTemporary(5, /*buffer_bytes=*/128);
    EXPECT_EQ(tmp->Size(), 5u);
    tmp->Write(42);
    EXPECT_NO_THROW(tmp->Read());
    EXPECT_NO_THROW(tmp->SetMemoryLimit(256));
}

TEST(FileTapeTest, OutOfRange) {
    const std::string fname = "test_tape4.bin";
    std::vector<int32_t> initial = {1};
    WriteIntFile(fname, initial);
    FileTape tape(fname, Delays{0,0,0,0});
    EXPECT_FALSE(tape.Prev());
    EXPECT_FALSE(tape.Next());
    EXPECT_FALSE(tape.Rewind(-2));
}

// Проверяем, что память не превышает заданный лимит
TEST(FileTapeTest, MemoryLimitEnforced) {
    const std::string fname = "test_tape_mem.bin";
    std::vector<int32_t> data(1e5, 52);
    
    WriteIntFile(fname, data);
    
    FileTape tape(fname, Delays{0,0,0,0});
    std::size_t start = current_alloc; // data, filename 

    for (size_t limit = 512; limit >= 4; limit /= 2) {
      max_alloc = 0;
      tape.SetMemoryLimit(limit);

      for (size_t pos = 0; pos < data.size() - 10; ++pos) {
        tape.Read();
        tape.Next();
        tape.Write(42);
        tape.Prev();
        tape.Rewind(2);
        tape.Read();
        tape.Rewind(-1);
      }

      tape.Reset();

      std::cout << "Limit: " << limit << std::endl;
      std::cout << "Used:  " << max_alloc - start << std::endl;
      EXPECT_LE(max_alloc - start, limit);
    }
}
