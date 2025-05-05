#pragma once

#include "delays.hpp"

#include <cstddef>
#include <cstdint>

#include <optional>
#include <string>

/// Настройки для FileTape и алгоритмов сортировки, загружаемые из YAML-конфига
struct Config {
    Delays delays;

    std::size_t memory_limit_bytes;

    // false => std::sort для сортировки чанков, глубина стека - O(log n)
    // true  => heap_sort для сортировки чанков, глубина стека - O(1)
    bool strict_stack_limit;

    // Диапазон значений для Counting Sort (опционально)
    std::optional<int32_t> value_min;
    std::optional<int32_t> value_max;

    static Config Load(const std::string& config_path);
};