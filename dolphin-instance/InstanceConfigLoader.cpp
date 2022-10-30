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
    std::string userRoot = File::GetUserPath(D_USER_IDX) + "../Temp";
    config_layer->Set(Config::MAIN_MEMCARD_A_PATH, userRoot + "/" + _memoryCardName + "_A.raw");
    config_layer->Set(Config::MAIN_MEMCARD_B_PATH, userRoot + "/" + _memoryCardName + "_B.raw");
}

void InstanceConfigLoader::Save(Config::Layer* config_layer)
{
}

// Loader generation
std::unique_ptr<Config::ConfigLayerLoader> GenerateInstanceConfigLoader(std::string memoryCardName)
{
    return std::make_unique<InstanceConfigLoader>(memoryCardName);
}
