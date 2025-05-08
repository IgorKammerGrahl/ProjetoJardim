#include "config_parser.h"
#include <jsoncpp/json/json.h>
#include <fstream>
#include <iostream>

std::unordered_map<std::string, GameConfig> loadGameConfigs(const std::string& configPath) {
    std::unordered_map<std::string, GameConfig> configs;

    std::ifstream file(configPath);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir arquivo de configuração: " << configPath << std::endl;
        return configs;
    }

    Json::Value root;
    Json::CharReaderBuilder builder;
    if (!Json::parseFromStream(builder, file, &root, nullptr)) {
        std::cerr << "Erro ao parsear JSON" << std::endl;
        return configs;
    }

    for (const auto& game : root["games"]) {
        GameConfig config;
        config.executable = game["executable"].asString();
        config.subject = game["subject"].asString();
        config.description = game["description"].asString();

        for (const auto& skill : game["skills"])
            config.skills.push_back(skill.asString());

        configs[config.executable] = config;
    }

    return configs;
}
