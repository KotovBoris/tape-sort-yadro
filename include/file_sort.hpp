#pragma once

#include "config.hpp"
#include "external_sort.hpp"
#include "file_tape.hpp"

#include <cstdio>

#include <filesystem>
#include <iostream>
#include <string>

static const size_t kPrefixSize = 20; 

void PrintTape(Tape& tape, size_t limit = kPrefixSize) {
    auto old_pos = tape.Position();
    tape.Reset();

    if (limit < tape.Size()) {
        std::cerr << "First " << limit << " elements:\n";
    }

    for (size_t i = 0; i < tape.Size() && i < limit; ++i) {
        std::cerr << tape.Read() << " ";
        tape.Next();
    }

    tape.Reset();
    tape.Rewind(old_pos);

    std::cout << std::endl;
}

namespace ext_sort {
// Файл в формате FileTape - последовательно записанные int32
void FileSort(const std::string& input_file,
              const std::string& output_file,
              const std::string& config_file) {
    std::error_code ec;
    std::filesystem::create_directory("tmp", ec);
    if (ec) {
        std::cerr << "Warning: Could not create 'tmp' directory: " << ec.message() << "\n";
    }

    Config cfg = Config::Load(config_file);

    FileTape input_tape(input_file, cfg.delays);
    std::cerr << "Input tape is loaded:\n";
    PrintTape(input_tape);
    std::cerr << "\n";


    std::size_t n = input_tape.Size();
    std::FILE* out_f = std::fopen(output_file.c_str(), "wb");
    if (!out_f) {
        throw std::runtime_error("Failed to create output file: " + output_file);
    }
    if (n > 0) {
        if (std::fseek(out_f, static_cast<long>(n * sizeof(int32_t) - 1), SEEK_SET) != 0 ||
            std::fputc(0, out_f) == EOF) {
            std::fclose(out_f);
            throw std::runtime_error("Failed to allocate space for output file");
        }
    }
    std::fclose(out_f);

    
    FileTape output_tape(output_file, cfg.delays);

    // Выбор алгоритма сортировки
    std::cerr << "Selected sorting algorithm: ";
    if (cfg.value_min.has_value() && cfg.value_max.has_value()) {
        std::cerr << "Counting Sort\n\n";
        std::cerr << "Starting sorting...\n\n";

        ext_sort::CountingSort(
            input_tape,
            output_tape,
            cfg.memory_limit_bytes,
            *cfg.value_min,
            *cfg.value_max
        );
    } else {
        std::cerr << "Chunk Merge Sort\n\n";
        std::cerr << "Starting sorting...\n\n";

        ext_sort::ChunkMergeSort(
            input_tape,
            output_tape,
            cfg.memory_limit_bytes,
            cfg.strict_stack_limit
        );
    }

    std::cerr << "Result: " << output_file << "\n";
    PrintTape(output_tape);
}

} // namespace ext_sort