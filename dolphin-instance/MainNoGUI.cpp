// Copyright 2008 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

// Compile fix for debug mode
#undef FMT_USE_NONTYPE_TEMPLATE_PARAMETERS
#define FMT_USE_NONTYPE_TEMPLATE_PARAMETERS 0

#undef USE_DISCORD_PRESENCE

#include "Instance.h"

#include "GBAInstance.h"

#include <OptionParser.h>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <signal.h>
#include <string>
#include <vector>

#ifndef _WIN32
#include <unistd.h>
#else
#include <Windows.h>
#endif

#include "Common/StringUtil.h"
#include "Core/Boot/Boot.h"
#include "Core/BootManager.h"
#include "Core/Core.h"
#include "Core/DolphinAnalytics.h"
#include "Core/Host.h"

#include "UICommon/CommandLineParse.h"
#ifdef USE_DISCORD_PRESENCE
#include "UICommon/DiscordPresence.h"
#endif
#include "UICommon/UICommon.h"

#include "InputCommon/GCAdapter.h"

#include "VideoCommon/RenderBase.h"
#include "VideoCommon/VideoBackendBase.h"

static std::shared_ptr<Instance> PlatformInstance;

static void signal_handler(int)
{
    const char message[] = "A signal was received. A second signal will force Dolphin to stop.\n";
    #ifdef _WIN32
        puts(message);
    #else
        if (write(STDERR_FILENO, message, sizeof(message)) < 0)
        {
        }
    #endif

    PlatformInstance->RequestShutdown();
}

std::vector<std::string> Host_GetPreferredLocales()
{
    return {};
}

void Host_NotifyMapLoaded()
{
}

void Host_RefreshDSPDebuggerWindow()
{
}

bool Host_UIBlocksControllerState()
{
    return false;
}

static Common::Event s_update_main_frame_event;
void Host_Message(HostMessageID id)
{
    if (id == HostMessageID::WMUserStop)
    {
        PlatformInstance->Stop();
    }
}

void Host_UpdateTitle(const std::string& title)
{
    // TODO: This is crashy with save states (which call this func), figure out why
    /*
    if (PlatformInstance)
    {
        PlatformInstance->SetTitle(title);
    }*/
}

void Host_UpdateDisasmDialog()
{
}

void Host_UpdateMainFrame()
{
    s_update_main_frame_event.Set();
}

void Host_RequestRenderWindowSize(int width, int height)
{
}

bool Host_RendererHasFocus()
{
    return PlatformInstance->IsWindowFocused();
}

bool Host_RendererHasFullFocus()
{
    // Mouse capturing isn't implemented
    return Host_RendererHasFocus();
}

bool Host_RendererIsFullscreen()
{
    return PlatformInstance->IsWindowFullscreen();
}

void Host_YieldToUI()
{
}

void Host_TitleChanged()
{
#ifdef USE_DISCORD_PRESENCE
    Discord::UpdateDiscordPresence();
#endif
}

std::unique_ptr<GBAHostInterface> Host_CreateGBAHost(std::weak_ptr<HW::GBA::Core> core)
{
    return std::make_unique<GBAInstance>(core, PlatformInstance);
}

static std::unique_ptr<Instance> GetInstance(const optparse::Values& options)
{
    std::string platformName = static_cast<const char*>(options.get("platform"));
    std::string instanceId = static_cast<const char*>(options.get("instanceId"));

    if (instanceId.empty())
    {
        instanceId = "MOCK";
    }

    InstanceBootParameters params;

    params.instanceId = instanceId;
    params.recordOnLaunch = options.is_set("record");
    params.pauseOnBoot = options.is_set("pause");

    #if HAVE_X11
        if (platformName == "x11" || platformName.empty())
            return Instance::CreateX11Instance(params);
    #endif

    #ifdef __linux__
        if (platformName == "fbdev" || platformName.empty())
            return Instance::CreateFBDevInstance(params);
    #endif

    #ifdef _WIN32
        if (platformName == "win32" || platformName.empty())
            return Instance::CreateWin32Instance(params);
    #endif

    if (platformName == "headless" || platformName.empty())
        return Instance::CreateHeadlessInstance(params);

    return nullptr;
}

#ifdef _WIN32
#define main app_main
#endif

std::unique_ptr<optparse::OptionParser> createParser()
{
    auto parser = CommandLineParse::CreateParser(CommandLineParse::ParserOptions::OmitGUIOptions);
    parser->add_option("-p", "--platform")
        .action("store")
        .help("Window platform to use [%choices]")
        .choices({
        "headless"
        #ifdef __linux__
        , "fbdev"
        #endif
        #if HAVE_X11
        , "x11"
        #endif
        #ifdef _WIN32
        , "win32"
        #endif
    });

    parser->add_option("-i", "--instanceId")
        .action("append")
        .metavar("<id>")
        .type("string")
        .help("A unique instance identifier used for creating IPC channels.");

    parser->add_option("-r", "--record").action("store_true").help("Start recording input on launch");
    parser->add_option("-z", "--pause").action("store_true").help("Pause emulation on launch");

    return parser;
}

int main(int argc, char* argv[])
{
    std::unique_ptr<optparse::OptionParser> parser = createParser();
    optparse::Values& options = CommandLineParse::ParseArguments(parser.get(), argc, argv);
    std::vector<std::string> args = parser->args();

    std::optional<std::string> save_state_path;
    if (options.is_set("save_state"))
    {
        save_state_path = static_cast<const char*>(options.get("save_state"));
    }

    // TODO: Temp debug! Remove this at some point
    if (args.size() <= 0)
    {
        args.push_back("C:/Dolphin/Games/Star Fox Adventures (USA) (v1.00).iso");
    }

    std::unique_ptr<BootParameters> boot;
    bool game_specified = false;
    if (options.is_set("exec"))
    {
        const std::list<std::string> paths_list = options.all("exec");
        const std::vector<std::string> paths{std::make_move_iterator(std::begin(paths_list)),
        std::make_move_iterator(std::end(paths_list))};
        boot = BootParameters::GenerateFromFile(
        paths, BootSessionData(save_state_path, DeleteSavestateAfterBoot::No));
        game_specified = true;
    }
    else if (options.is_set("nand_title"))
    {
        const std::string hex_string = static_cast<const char*>(options.get("nand_title"));
        if (hex_string.length() != 16)
        {
            fprintf(stderr, "Invalid title ID\n");
            parser->print_help();
            return 1;
        }

        const u64 title_id = std::stoull(hex_string, nullptr, 16);
        boot = std::make_unique<BootParameters>(BootParameters::NANDTitle{title_id});
    }
    else if (args.size() > 0)
    {
        boot = BootParameters::GenerateFromFile(args.front(), BootSessionData(save_state_path, DeleteSavestateAfterBoot::No));
        args.erase(args.begin());
        game_specified = true;
    }
    else
    {
        parser->print_help();
        return 0;
    }

    std::string user_directory;
    if (options.is_set("user"))
    {
        user_directory = static_cast<const char*>(options.get("user"));
    }

    UICommon::SetUserDirectory(user_directory);
    UICommon::Init();
    GCAdapter::Init();

    PlatformInstance = GetInstance(options);
    if (!PlatformInstance || !PlatformInstance->Init())
    {
        fprintf(stderr, "No platform found, or failed to initialize.\n");
        return 1;
    }

    if (save_state_path && !game_specified)
    {
        fprintf(stderr, "A save state cannot be loaded without specifying a game to launch.\n");
        return 1;
    }

    Core::AddOnStateChangedCallback([](Core::State state)
    {
        if (state == Core::State::Uninitialized)
        {
            PlatformInstance->Stop();
        }
    });

    #ifdef _WIN32
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
    #else
        // Shut down cleanly on SIGINT and SIGTERM
        struct sigaction sa;
        sa.sa_handler = signal_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESETHAND;
        sigaction(SIGINT, &sa, nullptr);
        sigaction(SIGTERM, &sa, nullptr);
    #endif

    DolphinAnalytics::Instance().ReportDolphinStart("nogui");

    if (!BootManager::BootCore(std::move(boot), PlatformInstance->GetWindowSystemInfo()))
    {
        fprintf(stderr, "Could not boot the specified file\n");
        return 1;
    }

    #ifdef USE_DISCORD_PRESENCE
        Discord::UpdateDiscordPresence();
    #endif

    PlatformInstance->MainLoop();
    Core::Stop();

    Core::Shutdown();
    PlatformInstance.reset();
    UICommon::Shutdown();

    return 0;
}

#ifdef _WIN32
int wmain(int, wchar_t*[], wchar_t*[])
{
    std::vector<std::string> args = CommandLineToUtf8Argv(GetCommandLineW());
    const int argc = static_cast<int>(args.size());
    std::vector<char*> argv(args.size());
    for (size_t i = 0; i < args.size(); ++i)
    {
        argv[i] = args[i].data();
    }

    return main(argc, argv.data());
}

#undef main
#endif
