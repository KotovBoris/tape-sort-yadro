#include "external_sort.hpp"

#include "tape.hpp"

#include <cstdint>

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace {

// 2 ленты: в одной последовательно записаны чанки с четными номерами
//          в другой - с нечетными
struct Chunks {
    std::unique_ptr<Tape> even_tape;
    std::unique_ptr<Tape> odd_tape;
    std::size_t chunk_length;
    std::size_t total_size;

    void Swap(Chunks& other) {
        even_tape.swap(other.even_tape);
        odd_tape.swap(other.odd_tape);
        std::swap(chunk_length, other.chunk_length);
        std::swap(total_size, other.total_size);
    }
};

Chunks sortChunks(
    Tape& input,
    std::size_t memory_limit_bytes,
    bool use_heap_sort
) {
    input.Reset();

    // Распределяем память: половина на сортировку, оставшееся - на три ленты
    std::size_t sort_buffer = std::max(memory_limit_bytes / 2, sizeof(int32_t));
    std::size_t remaining = memory_limit_bytes - sort_buffer;
    std::size_t buffer_per_tape = remaining / 3;

    input.SetMemoryLimit(buffer_per_tape);

    std::size_t total = input.Size();
    std::size_t max_elements = sort_buffer / sizeof(int32_t);
    if (max_elements == 0) {
        throw std::runtime_error("Sort buffer too small for even one element");
    }

    std::vector<int32_t> buffer;
    buffer.reserve(max_elements);

    Chunks chunks{
        input.CreateTemporary(total, buffer_per_tape),
        input.CreateTemporary(total, buffer_per_tape),
        max_elements,
        total
    };

    bool write_to_even = true;
    std::size_t processed = 0;
    while (processed < total) {
        buffer.clear();
        std::size_t chunk_size = std::min(max_elements, total - processed);

        for (std::size_t i = 0; i < chunk_size; ++i) {
            buffer.push_back(input.Read());

            input.Next();
        }

        if (use_heap_sort) {
            std::make_heap(buffer.begin(), buffer.end());
            std::sort_heap(buffer.begin(), buffer.end());
        } else {
            std::sort(buffer.begin(), buffer.end());
        }

        Tape* dest = write_to_even ? chunks.even_tape.get() : chunks.odd_tape.get();
        for (std::size_t i = 0; i < buffer.size(); ++i) {
            dest->Write(buffer[i]);
            dest->Next();
        }

        write_to_even = !write_to_even;
        processed += chunk_size;
    }

    input.SetMemoryLimit(0);
    return chunks;
}

// Одна итерация: увеличиваем размер чанков в 2 раза
void mergeIteration(
    Chunks& in,
    Chunks& out
) {
    in.even_tape->Reset();
    in.odd_tape->Reset();
    out.even_tape->Reset();
    out.odd_tape->Reset();

    bool write_to_even = true;
    std::size_t processed = 0;

    Tape* curr_even = in.even_tape.get();
    Tape* curr_odd  = in.odd_tape.get();

    while (processed < in.total_size) {
        std::size_t left_size  = std::min(in.chunk_length, in.total_size - processed);
        std::size_t right_size = std::min(in.chunk_length, in.total_size - processed - left_size);

        std::size_t left_index = 0;
        std::size_t right_index = 0;
        int32_t left_value = left_size ? curr_even->Read() : 0;
        int32_t right_value = right_size ? curr_odd->Read() : 0;

        Tape* dest = write_to_even ? out.even_tape.get() : out.odd_tape.get();
        while (left_index < left_size || right_index < right_size) {
            if (left_index < left_size && (right_index >= right_size || left_value <= right_value)) {
                dest->Write(left_value);
                ++left_index;

                curr_even->Next();

                if (left_index < left_size) {
                    left_value = curr_even->Read();
                }
            } else {
                dest->Write(right_value);
                ++right_index;

                curr_odd->Next();
                
                if (right_index < right_size) {
                    right_value = curr_odd->Read();
                }
            }
            ++processed;
            dest->Next();
        }
        write_to_even = !write_to_even;
    }

    out.chunk_length = in.chunk_length * 2;
}

void mergeAllChunks(
    Chunks chunks,
    Tape& output,
    std::size_t memory_limit_bytes
) {
    // Распределяем память на пять лент: текущие две, новые две, и выход
    std::size_t per_buffer = memory_limit_bytes / 5;
    chunks.even_tape->SetMemoryLimit(per_buffer);
    chunks.odd_tape->SetMemoryLimit(per_buffer);
    output.SetMemoryLimit(per_buffer);

    // Создаём структуры для текущей и следующей фаз
    Chunks current = std::move(chunks);
    Chunks next{
        current.even_tape->CreateTemporary(current.total_size, per_buffer),
        current.odd_tape->CreateTemporary(current.total_size, per_buffer),
        current.chunk_length,
        current.total_size
    };
    next.even_tape->SetMemoryLimit(per_buffer);
    next.odd_tape->SetMemoryLimit(per_buffer);

    while (current.chunk_length < current.total_size) {
        current.even_tape->Reset();
        current.odd_tape->Reset();
        next.even_tape->Reset();
        next.odd_tape->Reset();

        mergeIteration(current, next);

        current.Swap(next);
    }

    // Финальная запись в выходную ленту
    current.even_tape->Reset();
    output.Reset();
    for (std::size_t i = 0; i < current.total_size; ++i) {
        output.Write(current.even_tape->Read());
        current.even_tape->Next();
        output.Next();
    }
}

} // namespace

namespace ext_sort {

void ChunkMergeSort(
    Tape& input,
    Tape& output,
    std::size_t memory_limit_bytes,
    bool use_heap_sort
) {
    if (memory_limit_bytes < sizeof(int32_t)) {
        throw std::runtime_error("Memory limit too small for even one element");
    }

    Chunks chunks = sortChunks(input, memory_limit_bytes, use_heap_sort);
    mergeAllChunks(std::move(chunks), output, memory_limit_bytes);
    
    output.Reset();
}

} // namespace ext_sort
