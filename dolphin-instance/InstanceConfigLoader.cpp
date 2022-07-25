#include "InstanceConfigLoader.h"

#include <cstring>
#include <memory>
#include <string>

#include "Common/CommonTypes.h"
#include "Common/Config/Config.h"

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

void InstanceConfigLoader::Load(Config::Layer* config_layer)
{
    /*
    
    config_layer->Set(Config::MAIN_CPU_THREAD, dtm->bDualCore);
    config_layer->Set(Config::MAIN_DSP_HLE, dtm->bDSPHLE);
    config_layer->Set(Config::MAIN_FAST_DISC_SPEED, dtm->bFastDiscSpeed);
    config_layer->Set(Config::MAIN_CPU_CORE, static_cast<PowerPC::CPUCore>(dtm->CPUCore));
    config_layer->Set(Config::MAIN_SYNC_GPU, dtm->bSyncGPU);
    config_layer->Set(Config::MAIN_GFX_BACKEND, dtm->videoBackend.data());

    config_layer->Set(Config::SYSCONF_PROGRESSIVE_SCAN, dtm->bProgressive);
    config_layer->Set(Config::SYSCONF_PAL60, dtm->bPAL60);
    if (dtm->bWii)
        config_layer->Set(Config::SYSCONF_LANGUAGE, static_cast<u32>(dtm->language));
    else
        config_layer->Set(Config::MAIN_GC_LANGUAGE, static_cast<int>(dtm->language));

    config_layer->Set(Config::GFX_HACK_EFB_ACCESS_ENABLE, dtm->bEFBAccessEnable);
    config_layer->Set(Config::GFX_HACK_SKIP_EFB_COPY_TO_RAM, dtm->bSkipEFBCopyToRam);
    config_layer->Set(Config::GFX_HACK_EFB_EMULATE_FORMAT_CHANGES, dtm->bEFBEmulateFormatChanges);
    config_layer->Set(Config::GFX_HACK_IMMEDIATE_XFB, dtm->bImmediateXFB);
    config_layer->Set(Config::GFX_HACK_SKIP_XFB_COPY_TO_RAM, dtm->bSkipXFBCopyToRam);

    config_layer->Set(Config::SESSION_USE_FMA, dtm->bUseFMA);

    config_layer->Set(Config::MAIN_JIT_FOLLOW_BRANCH, dtm->bFollowBranch);
    */
}

void InstanceConfigLoader::Save(Config::Layer* config_layer)
{
}

// Loader generation
std::unique_ptr<Config::ConfigLayerLoader> GenerateInstanceConfigLoader()
{
    return std::make_unique<InstanceConfigLoader>();
}
