#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <filesystem>  // Necessário para std::filesystem

struct GameConfig {
    std::string executable;
    std::string subject;
    std::vector<std::string> skills;
    std::string description;
};

struct GameInfo {
    std::filesystem::path path;
    GameConfig cfg;
};

std::unordered_map<std::string, GameConfig> loadGameConfigs(const std::string& configPath);
