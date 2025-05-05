#pragma once

#include "tape.hpp"

#include <cstddef>
#include <cstdint>

namespace ext_sort {

// Cортировка подсчётом без заранее известного диапазона
void CountingSort(Tape& input, Tape& output, std::size_t memory_limit_bytes);

// Сортировка подсчётом с заранее известным диапазоном [value_min, value_max]
void CountingSort(Tape& input, Tape& output,
                  std::size_t memory_limit_bytes,
                  int32_t value_min,
                  int32_t value_max);

// Сначала сортируем чанки с помощью heap/std sort
// После сливаем их, как в MergeSort
void ChunkMergeSort(Tape& input, Tape& output,
                    std::size_t memory_limit_bytes,
                    bool use_heap_sort);

} // namespace ext_sort