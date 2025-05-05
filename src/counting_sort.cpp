#include "external_sort.hpp"

#include "tape.hpp"

#include <cstdint>

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace {
    constexpr std::size_t MAX_TAPE_BUFFER_BYTES = 128ull * 1024 * 1024;

    // Количество памяти, которое будет отведено на каждую ленту (входную и выходную)
    std::size_t chooseTapeBufferSize(std::size_t memory_limit_bytes) {
        std::size_t quarter = memory_limit_bytes / 4;
        return std::min(quarter, MAX_TAPE_BUFFER_BYTES);
    }

    // Подсчёт элементов в диапазоне [win_start, win_end]
    std::vector<std::size_t> countWindow(Tape& tape,
                                         std::size_t total_elems,
                                         int32_t win_start,
                                         int32_t win_end) {
        std::vector<std::size_t> counts(static_cast<std::size_t>(win_end - win_start + 1), 0);

        tape.Reset();
        for (std::size_t i = 0; i < total_elems; ++i) {
            int32_t v = tape.Read();
            if (v >= win_start && v <= win_end) {
                counts[static_cast<std::size_t>(v - win_start)]++;
            }
            tape.Next();
        }

        return counts;
    }

    // Запись cnt значений value на ленту
    void writeCount(Tape& tape, int32_t value, std::size_t cnt) {
        for (std::size_t c = 0; c < cnt; ++c) {
            tape.Write(value);
            tape.Next();
        }
    }
} // namespace

namespace ext_sort {

// Сортировка подсчётом с известным диапазоном
void CountingSort(
    Tape& input,
    Tape& output,
    std::size_t memory_limit_bytes,
    int32_t global_min,
    int32_t global_max
) {
    std::size_t n = input.Size();
    if (n == 0) {
        return;
    }

    std::size_t buf = chooseTapeBufferSize(memory_limit_bytes);
    input.SetMemoryLimit(buf);
    output.SetMemoryLimit(buf);

    std::size_t count_buf = memory_limit_bytes - 2 * buf;
    if (count_buf < sizeof(std::size_t)) {
        throw std::runtime_error("Memory limit too small for counting sort buffer");
    }

    uint64_t total_range = static_cast<uint64_t>(global_max) - static_cast<uint64_t>(global_min) + 1;
    std::size_t max_counts = count_buf / sizeof(std::size_t);
    if (max_counts == 0) {
        throw std::runtime_error("Memory limit too small for counting sort buffer");
    }
    std::size_t window_size = static_cast<std::size_t>(std::min(total_range, static_cast<uint64_t>(max_counts)));

    std::size_t tmp = output.Size();

    output.Reset();
    for (int32_t start = global_min; start <= global_max; start += static_cast<int32_t>(window_size)) {
        int32_t end = std::min(global_max, start + static_cast<int32_t>(window_size) - 1);

        std::vector<std::size_t> counts = countWindow(input, n, start, end);

        for (std::size_t i = 0; i < counts.size(); ++i) {
            writeCount(output, start + static_cast<int32_t>(i), counts[i]);
        }
    }

    output.Reset();
}

// Сортировка подсчётом с неизвестным диапазоном
void CountingSort(
    Tape& input,
    Tape& output,
    std::size_t memory_limit_bytes
) {
    if (input.Size() == 0) {
        return;
    }

    input.Reset();
    int32_t global_min = input.Read();
    int32_t global_max = global_min;
    std::size_t total = input.Size();
    for (std::size_t i = 1; i < total; ++i) {
        input.Next();
        int32_t v = input.Read();
        global_min = std::min(global_min, v);
        global_max = std::max(global_max, v);
    }

    CountingSort(input, output, memory_limit_bytes, global_min, global_max);
}

} // namespace ext_sort