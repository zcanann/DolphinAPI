#pragma once

#include <memory>

#include "Common/Config/Config.h"

class InstanceConfigLoader final : public Config::ConfigLayerLoader
{
public:
    explicit InstanceConfigLoader()
        : ConfigLayerLoader(Config::LayerType::CurrentRun)
    {
    }

    void Load(Config::Layer* config_layer) override;
    void Save(Config::Layer* config_layer) override;

private:
};

void SaveToDTM();
std::unique_ptr<Config::ConfigLayerLoader> GenerateInstanceConfigLoader();
