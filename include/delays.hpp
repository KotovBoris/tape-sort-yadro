#pragma once

#include <cstddef>

/// Задержки для операций с лентой (в миллисекундах)
struct Delays {
    std::size_t read_ms;
    std::size_t write_ms;
    std::size_t shift_ms;        // сдвиг на одну ячейку
    std::size_t rewind_ms;       // перемотка на произвольное число ячеек
};
