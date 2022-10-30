#pragma once

#include <memory>

#include "Common/Config/Config.h"

class InstanceConfigLoader final : public Config::ConfigLayerLoader
{
public:
    InstanceConfigLoader(std::string memoryCardName);

    void Load(Config::Layer* config_layer) override;
    void Save(Config::Layer* config_layer) override;

private:
    std::string _memoryCardName;
};

void SaveToDTM();
std::unique_ptr<Config::ConfigLayerLoader> GenerateInstanceConfigLoader(std::string memoryCardName);
