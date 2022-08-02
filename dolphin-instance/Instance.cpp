// Copyright 2022 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Instance.h"

#include "Input.h"
#include "InstanceConfigLoader.h"
#include "MockServer.h"
#include "TemplateHelpers.h"

// Dolphin includes
#include "Common/FileUtil.h"
#include "Core/Config/MainSettings.h"
#include "Core/Config/WiimoteSettings.h"
#include "Core/ConfigManager.h"
#include "Core/Core.h"
#include "Core/HW/CPU.h"
#include "Core/HW/Wiimote.h"
#include "Core/IOS/IOS.h"
#include "Core/IOS/STM/STM.h"
#include "Core/State.h"
#include "Core/HW/DVD/DVDInterface.h"
#include "Core/HW/EXI/EXI_DeviceIPL.h"
#include "Core/HW/GBAPad.h"
#include "Core/HW/GCKeyboard.h"
#include "Core/HW/GCMemcard/GCMemcard.h"
#include "Core/HW/GCPad.h"
#include "Core/HW/ProcessorInterface.h"
#include "Core/HW/SI/SI_Device.h"
#include "Core/HW/Sram.h"
#include "Core/HW/VideoInterface.h"
#include "Core/HW/Wiimote.h"
#include "Core/HW/WiimoteEmu/WiimoteEmu.h"
#include "InputCommon/ControllerInterface/ControllerInterface.h"
#include "InputCommon/GCAdapter.h"
#include "InputCommon/GCPadStatus.h"
#include "InputCommon/InputConfig.h"
#include "VideoCommon/VideoConfig.h"

#pragma optimize("", off)

Instance::Instance(const InstanceBootParameters& bootParams)
{
    initializeChannels(bootParams.instanceId, true);
    InitializeLaunchOptions(bootParams);

    Common::Log::LogManager::GetInstance()->RegisterListener(Common::Log::LogListener::LOG_WINDOW_LISTENER, this);
}

Instance::~Instance()
{
}

void Instance::InitializeLaunchOptions(const InstanceBootParameters& bootParams)
{
    bool recordOnLaunch = bootParams.recordOnLaunch;

    // For debugging some parts of IPC locally
    if (bootParams.instanceId == "MOCK")
    {
        _mockServer = std::make_shared<MockServer>(bootParams.instanceId);
        recordOnLaunch = true;

        for (int index = 0; index < 800000; index++)
        {
            _recordingInputs.push_back(DolphinControllerState());
        }
    }
    else
    {
        // Overwrite certain configs for non-mock sessions
        Config::AddLayer(GenerateInstanceConfigLoader());
    }

    if (recordOnLaunch)
    {
        StartRecording();
    }

    if (bootParams.pauseOnBoot)
    {
        SConfig::GetInstance().bBootToPause = bootParams.pauseOnBoot;
    }
}

void Instance::SetTitle(const std::string& title)
{
}

bool Instance::Init()
{
    // Ipc post-connect callback
    DolphinIpcToServerData ipcData;
    std::shared_ptr<ToServerParams_OnInstanceConnected> data = std::make_shared<ToServerParams_OnInstanceConnected>();
    ipcData._call = DolphinServerIpcCall::DolphinServer_OnInstanceConnected;
    ipcData._params._paramsOnInstanceConnected = data;
    ipcSendToServer(ipcData);

    // Ipc post-ready callback
    _coreStateEventHandle = Core::AddOnStateChangedCallback([this](Core::State state)
    {
        if (state == Core::State::Running || state == Core::State::Paused)
        {
            OnCommandCompleted(DolphinInstanceIpcCall::DolphinInstance_Connect);
            Core::RemoveOnStateChangedCallback(&_coreStateEventHandle);
        }
    });


    InitControllers();
    PrepareForTASInput();

    return true;
}

void Instance::InitControllers()
{
    if (g_controller_interface.IsInit())
    {
        return;
    }

    g_controller_interface.Initialize(GetWindowSystemInfo());
    GCAdapter::Init();
    Pad::Initialize();
    Pad::InitializeGBA();
    Keyboard::Initialize();
    Wiimote::Initialize(Wiimote::InitializeMode::DO_NOT_WAIT_FOR_WIIMOTES);
}

void Instance::ShutdownControllers()
{
    Pad::Shutdown();
    Pad::ShutdownGBA();
    Keyboard::Shutdown();
    Wiimote::Shutdown();
    g_controller_interface.Shutdown();
}

void Instance::PrepareForTASInput()
{
    Wiimote::ResetAllWiimotes();
    Core::UpdateWantDeterminism();
    Movie::SetReadOnly(false);

    Movie::SetGCInputManip([this](GCPadStatus* padStatus, int controllerId)
    {
        switch (_instanceState)
        {
            case RecordingState::FrameAdvancing:
            {
                if (_framesToAdvance <= 0)
                {
                    // Should never happen
                    return;
                }

                if (_frameAdvanceInput.has_value())
                {
                    Input::CopyControllerStateToGcPadStatus(*_frameAdvanceInput, padStatus);
                }

                if (_framesToAdvance-- <= 0)
                {
                    if (_frameAdvanceInput.has_value())
                    {
                        _frameAdvanceInput.reset();
                        OnCommandCompleted(DolphinInstanceIpcCall::DolphinInstance_FrameAdvanceWithInput);
                    }
                    else
                    {
                        OnCommandCompleted(DolphinInstanceIpcCall::DolphinInstance_FrameAdvance);
                    }
                }
                break;
            }
            case RecordingState::Playback:
            {
                if (_playbackInputs.size() <= 0)
                {
                    // Should never happen
                    return;
                }

                DolphinControllerState padState = _playbackInputs.front();
                _playbackInputs.erase(_playbackInputs.begin());

                Input::CopyControllerStateToGcPadStatus(padState, padStatus);

                if (padState.Disc)
                {
                    Core::RunAsCPUThread([]
                        {
                            if (!DVDInterface::AutoChangeDisc())
                            {
                                CPU::Break();
                                // PanicAlertFmtT("Change the disc to {0}", s_discChange);
                            }
                        });
                }

                if (padState.Reset)
                {
                    ProcessorInterface::ResetButton_Tap();
                }
                
                // Inputs complete! Ready for next command
                if (_playbackInputs.size() <= 0)
                {
                    Core::QueueHostJob([=] { Core::UpdateWantDeterminism(false); });
                    _instanceState = RecordingState::None;
                    OnCommandCompleted(DolphinInstanceIpcCall::DolphinInstance_PlayInputs);
                }
                break;
            }
            case RecordingState::Recording:
            {
                DolphinControllerState padState;
                Input::CopyGcPadStatusToControllerState(padStatus, padState);
                _recordingInputs.push_back(padState);
                break;
            }
            case RecordingState::None:
            default:
            {
                break;
            }
        }
    });

    /*
    Movie::ControllerTypeArray controllers { };
    Movie::WiimoteEnabledArray wiimotes { };

    for (int index = 0; index < 4; index++)
    {
        const SerialInterface::SIDevices si_device = Config::Get(Config::GetInfoForSIDevice(index));

        if (si_device == SerialInterface::SIDEVICE_GC_GBA_EMULATED)
        {
            controllers[index] = Movie::ControllerType::GBA;
        }
        else if (SerialInterface::SIDevice_IsGCController(si_device))
        {
            controllers[index] = Movie::ControllerType::GC;
        }
        else
        {
            controllers[index] = Movie::ControllerType::None;
        }

        wiimotes[index] = Config::Get(Config::GetInfoForWiimoteSource(index)) != WiimoteSource::None;
    }

    // TODO: Potentially enable this if we want to support dumping DTM files
    // Movie::BeginRecordingInput(controllers, wiimotes);*/
}

INSTANCE_FUNC_BODY(Instance, Connect, params)
{
}

INSTANCE_FUNC_BODY(Instance, Heartbeat, params)
{
    _lastHeartbeat = std::chrono::system_clock::now();

    // Acknowledge heartbeat, sending over any state data the server may want.
    DolphinIpcToServerData ipcData;
    std::shared_ptr<ToServerParams_OnInstanceHeartbeatAcknowledged> data = std::make_shared<ToServerParams_OnInstanceHeartbeatAcknowledged>();
    data->_isRecording = _instanceState == RecordingState::Recording;
    data->_isPaused = Core::GetState() == Core::State::Paused;
    ipcData._call = DolphinServerIpcCall::DolphinServer_OnInstanceHeartbeatAcknowledged;
    ipcData._params._paramsOnInstanceHeartbeatAcknowledged = data;
    ipcSendToServer(ipcData);
}

INSTANCE_FUNC_BODY(Instance, Terminate, params)
{
    StopRecording();
    RequestShutdown();
}

INSTANCE_FUNC_BODY(Instance, StartRecordingInput, params)
{
    StopRecording();
    StartRecording();
    OnCommandCompleted(DolphinInstanceIpcCall::DolphinInstance_StartRecordingInput);
}

INSTANCE_FUNC_BODY(Instance, StopRecordingInput, params)
{
    StopRecording();
    OnCommandCompleted(DolphinInstanceIpcCall::DolphinInstance_StopRecordingInput);
}

INSTANCE_FUNC_BODY(Instance, PauseEmulation, params)
{
    if (Core::GetState() == Core::State::Running)
    {
        Core::SetState(Core::State::Paused);
    }

    OnCommandCompleted(DolphinInstanceIpcCall::DolphinInstance_PauseEmulation);
}

INSTANCE_FUNC_BODY(Instance, ResumeEmulation, params)
{
    if (Core::GetState() == Core::State::Paused)
    {
        Core::SetState(Core::State::Running);
    }

    OnCommandCompleted(DolphinInstanceIpcCall::DolphinInstance_ResumeEmulation);
}

INSTANCE_FUNC_BODY(Instance, PlayInputs, params)
{
    // These vectors can be masive, use std::move to avoid an extra alloc (should be safe since _inputStates is not used after this)
    _playbackInputs = std::move(params._inputStates);

    if (Core::GetState() == Core::State::Paused)
    {
        Core::SetState(Core::State::Running);
    }

    if (_playbackInputs.size() <= 0)
    {
        OnCommandCompleted(DolphinInstanceIpcCall::DolphinInstance_PlayInputs);
        return;
    }

    _instanceState = RecordingState::Playback;
    // Send the initial flag to force determinism, since internally this checks Movie class flags which this class does not set
    Core::UpdateWantDeterminism(true);
}

INSTANCE_FUNC_BODY(Instance, FrameAdvance, params)
{
    _framesToAdvance = params._numFrames;
    _instanceState = RecordingState::FrameAdvancing;
}

INSTANCE_FUNC_BODY(Instance, FrameAdvanceWithInput, params)
{
    _framesToAdvance = params._numFrames;
    _instanceState = RecordingState::FrameAdvancing;
    _frameAdvanceInput = params._inputState;
}

INSTANCE_FUNC_BODY(Instance, CreateSaveState, params)
{
    if (!params._filePath.empty())
    {
        if (File::Exists(params._filePath))
        {
            File::Delete(params._filePath);
        }

        State::SaveAs(params._filePath, true);
    }

    DolphinIpcToServerData ipcData;
    std::shared_ptr<ToServerParams_OnInstanceSaveStateCreated> data = std::make_shared<ToServerParams_OnInstanceSaveStateCreated>();
    data->_filePath = params._filePath;
    data->_recordingInputs = _recordingInputs;
    ipcData._call = DolphinServerIpcCall::DolphinServer_OnInstanceSaveStateCreated;
    ipcData._params._paramsOnInstanceSaveStateCreated = data;
    ipcSendToServer(ipcData);

    OnCommandCompleted(DolphinInstanceIpcCall::DolphinInstance_CreateSaveState);
}

INSTANCE_FUNC_BODY(Instance, LoadSaveState, params)
{
    if (File::Exists(params._filePath))
    {
        State::LoadAs(params._filePath);
    }

    OnCommandCompleted(DolphinInstanceIpcCall::DolphinInstance_LoadSaveState);
}

INSTANCE_FUNC_BODY(Instance, FormatMemoryCard, params)
{
    std::string slotPath;
    switch (params._slot)
    {
        case DolphinSlot::SlotA:
        {
            slotPath = Config::Get(Config::MAIN_MEMCARD_A_PATH);
            break;
        }
        case DolphinSlot::SlotB:
        {
            slotPath = Config::Get(Config::MAIN_MEMCARD_A_PATH);
            break;
        }
        default:
        {
            return;
        }
    }

    if (!slotPath.empty())
    {
        if (File::Exists(slotPath))
        {
            File::Delete(slotPath);
        }

        u16 size;
        switch (params._cardSize)
        {
            case ToInstanceParams_FormatMemoryCard::CardSize::GC_4_Mbit_59_Blocks: size = 4; break;
            case ToInstanceParams_FormatMemoryCard::CardSize::GC_8_Mbit_123_Blocks: size = 8; break;
            case ToInstanceParams_FormatMemoryCard::CardSize::GC_16_Mbit_251_Blocks: size = 16; break;
            case ToInstanceParams_FormatMemoryCard::CardSize::GC_32_Mbit_507_Blocks: size = 32; break;
            case ToInstanceParams_FormatMemoryCard::CardSize::GC_64_Mbit_1019_Blocks: size = 64; break;
            default: case ToInstanceParams_FormatMemoryCard::CardSize::GC_128_Mbit_2043_Blocks: size = 128; break;
        }
        bool isShiftJis = params._encoding == ToInstanceParams_FormatMemoryCard::CardEncoding::Japanese;

        const CardFlashId flash_id{};
        const u32 rtc_bias = 0;
        const u32 sram_language = 0;
        const u64 format_time = Common::Timer::GetLocalTimeSinceJan1970() - ExpansionInterface::CEXIIPL::GC_EPOCH;

        std::optional<Memcard::GCMemcard> memcard = Memcard::GCMemcard::Create(slotPath, flash_id, size, isShiftJis, rtc_bias, sram_language, format_time);

        if (memcard)
        {
            memcard->Save();
        }
    }

    // TODO: Does this event need to exist?
    DolphinIpcToServerData ipcData;
    std::shared_ptr<ToServerParams_OnInstanceMemoryCardFormatted> data = std::make_shared<ToServerParams_OnInstanceMemoryCardFormatted>();
    data->_slot = params._slot;
    ipcData._call = DolphinServerIpcCall::DolphinServer_OnInstanceMemoryCardFormatted;
    ipcData._params._paramsOnInstanceMemoryCardFormatted = data;
    ipcSendToServer(ipcData);

    OnCommandCompleted(DolphinInstanceIpcCall::DolphinInstance_FormatMemoryCard);
}

void Instance::UpdateRunningFlag()
{
    updateIpcListen();

    if (_mockServer)
    {
        _mockServer->Update();
    }

    // Close if no heartbeat command sent over IPC recently
    if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - _lastHeartbeat) > std::chrono::seconds(15))
    {
        RequestShutdown();
    }

    if (_shutdown_requested.TestAndClear())
    {
        StopRecording();

        const auto ios = IOS::HLE::GetIOS();
        const auto stm = ios ? ios->GetDeviceByName("/dev/stm/eventhook") : nullptr;
        if (!_tried_graceful_shutdown.IsSet() && stm &&
            std::static_pointer_cast<IOS::HLE::STMEventHookDevice>(stm)->HasHookInstalled())
        {
            ProcessorInterface::PowerButton_Tap();
            _tried_graceful_shutdown.Set();
        }
        else
        {
            _running.Clear();
        }
    }
}

void Instance::StartRecording()
{
    if (_instanceState == RecordingState::Recording)
    {
        return;
    }

    _instanceState = RecordingState::Recording;
    Core::UpdateWantDeterminism(true);
}

void Instance::StopRecording()
{
    if (_instanceState != RecordingState::Recording)
    {
        return;
    }

    _instanceState = RecordingState::None;
    Core::UpdateWantDeterminism(false);

    DolphinIpcToServerData ipcData;
    std::shared_ptr<ToServerParams_OnInstanceRecordingStopped> data = std::make_shared<ToServerParams_OnInstanceRecordingStopped>();
    data->_inputStates = _recordingInputs;
    ipcData._call = DolphinServerIpcCall::DolphinServer_OnInstanceRecordingStopped;
    ipcData._params._paramsOnInstanceRecordingStopped = data;
    ipcSendToServer(ipcData);

    _recordingInputs.clear();
}

void Instance::OnCommandCompleted(DolphinInstanceIpcCall completedCommand)
{
    DolphinIpcToServerData ipcData;
    std::shared_ptr<ToServerParams_OnInstanceCommandCompleted> data = std::make_shared<ToServerParams_OnInstanceCommandCompleted>();
    data->_completedCommand = completedCommand;
    ipcData._call = DolphinServerIpcCall::DolphinServer_OnInstanceCommandCompleted;
    ipcData._params._paramsOnInstanceCommandCompleted = data;
    ipcSendToServer(ipcData);
}

void Instance::Log(Common::Log::LogLevel level, const char* text)
{
    // Intentionally using the same enum values so we can cast like this
    ToServerParams_OnInstanceLogOutput::LogLevel logLevel = (ToServerParams_OnInstanceLogOutput::LogLevel)level;
    std::string logString = std::string(text);

    DolphinIpcToServerData ipcData;
    std::shared_ptr<ToServerParams_OnInstanceLogOutput> data = std::make_shared<ToServerParams_OnInstanceLogOutput>();
    data->_logLevel = logLevel;
    data->_logString = logString;
    ipcData._call = DolphinServerIpcCall::DolphinServer_OnInstanceLogOutput;
    ipcData._params._paramsOnInstanceLogOutput = data;
    ipcSendToServer(ipcData);
}

void Instance::Stop()
{
    _running.Clear();
}

void Instance::RequestShutdown()
{
    _shutdown_requested.Set();
}

#pragma optimize("", on)
