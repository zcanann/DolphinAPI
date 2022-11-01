#include "InstanceConfigLoader.h"

#include <cstring>
#include <memory>
#include <string>

#include "Common/CommonTypes.h"
#include "Common/Config/Config.h"
#include "Common/FileUtil.h"
#include "Core/Config/GraphicsSettings.h"
#include "Core/Config/MainSettings.h"
#include "Core/Config/SYSCONFSettings.h"
#include "Core/Config/SessionSettings.h"
#include "Core/ConfigManager.h"
#include "Core/Movie.h"
#include "VideoCommon/VideoConfig.h"

namespace PowerPC
{
    enum class CPUCore;
}

InstanceConfigLoader::InstanceConfigLoader(std::string memoryCardName) : ConfigLayerLoader(Config::LayerType::CurrentRun)
{
    _memoryCardName = memoryCardName;
}

void InstanceConfigLoader::Load(Config::Layer* config_layer)
{
    // Use a custom (temp) memory card location, to prevent this instance from conflicting with other running instances
    // If we want to save any memory card data, its up to the user to save a copy of this data (probably via a Server UI => IPC call)
    std::string userRoot = File::GetUserPath(D_USER_IDX) + "../Temp";
    config_layer->Set(Config::MAIN_MEMCARD_A_PATH, userRoot + "/" + _memoryCardName + "_A.raw");
    config_layer->Set(Config::MAIN_MEMCARD_B_PATH, userRoot + "/" + _memoryCardName + "_B.raw");

    // Always allow background input
    config_layer->Set(Config::MAIN_INPUT_BACKGROUND_INPUT, true);
}

void InstanceConfigLoader::Save(Config::Layer* config_layer)
{
}

// Loader generation
std::unique_ptr<Config::ConfigLayerLoader> GenerateInstanceConfigLoader(std::string memoryCardName)
{
    return std::make_unique<InstanceConfigLoader>(memoryCardName);
}
