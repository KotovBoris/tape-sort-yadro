#include "config.hpp"

#include <yaml-cpp/yaml.h>

Config Config::Load(const std::string& path) {
    YAML::Node node = YAML::LoadFile(path);
    Config cfg;

    // Задержки
    cfg.delays.read_ms   = node["delays"]["read_ms"].as<std::size_t>();
    cfg.delays.write_ms  = node["delays"]["write_ms"].as<std::size_t>();
    cfg.delays.shift_ms  = node["delays"]["shift_ms"].as<std::size_t>();
    cfg.delays.rewind_ms = node["delays"]["rewind_ms"].as<std::size_t>();

    // Ограничения по памяти
    cfg.memory_limit_bytes = node["memory_limit_bytes"].as<std::size_t>();
    cfg.strict_stack_limit  = node["strict_stack_limit"].as<bool>();

    // Опциональный диапазон
    if (node["value_range"] && node["value_range"].IsSequence() && node["value_range"].size() == 2) {
        cfg.value_min = node["value_range"][0].as<int32_t>();
        cfg.value_max = node["value_range"][1].as<int32_t>();
    } else {
        cfg.value_min = std::nullopt;
        cfg.value_max = std::nullopt;
    }

    return cfg;
}