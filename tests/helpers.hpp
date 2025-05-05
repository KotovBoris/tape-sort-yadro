#include "tape.hpp"

#include <fstream>
#include <random>
#include <vector>

static std::vector<int32_t> TapeToVector(Tape& tape) {
  std::vector<int32_t> result;
  for (std::size_t i = 0; i < tape.Size(); ++i) {
      result.push_back(tape.Read());
      tape.Next();
  }
  return result;
}

static void WriteIntFile(const std::string& filename, const std::vector<int32_t>& data) {
  std::ofstream ofs(filename, std::ios::binary);
  for (auto v : data) {
    ofs.write(reinterpret_cast<const char*>(&v), sizeof(v));
  }
}

static std::vector<int32_t> ReadIntFile(const std::string& filename) {
  std::ifstream ifs(filename, std::ios::binary);
  std::vector<int32_t> data;
  int32_t v;
  while (ifs.read(reinterpret_cast<char*>(&v), sizeof(v))) {
      data.push_back(v);
  }
  return data;
}

static std::vector<int32_t> RandomVector(std::size_t n,
                                         int32_t min_value, int32_t max_value) {
  std::mt19937 gen(2025);
  std::uniform_int_distribution<int32_t> dist(min_value, max_value);

  std::vector<int32_t> data(n);
  for (int i = 0; i < n; ++i) {
    data[i] = dist(gen);
  }

  return data;
}

// Запись временного YAML-файла
static void WriteYaml(const std::string& path, const std::string& content) {
    std::ofstream ofs(path);
    ofs << content;
    ofs.close();
}
