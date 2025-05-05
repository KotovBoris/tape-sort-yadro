// src/main.cpp
#include "file_sort.hpp"

#include <iostream>
#include <string>
#include <filesystem>
#include <cstdio>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file> <config_file>\n";
        return 1;
    }

    std::cerr << "Path to input  FileTape: " << argv[1] << "\n";
    std::cerr << "Path to output FileTape: " << argv[2] << "\n";
    std::cerr << "Path to config file:     " << argv[3] << "\n\n";

    try {
        //                 input    output   config
        ext_sort::FileSort(argv[1], argv[2], argv[3]);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n\n";
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file> <config_file>\n";
        return 1;
    }

    return 0;
}