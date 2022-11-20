// Copyright 2022 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Instance.h"

#include "InstanceConfigLoader.h"
#include "InstanceUtils.h"
#include "MockServer.h"
#include "TemplateHelpers.h"

// Dolphin includes
#include "Common/FileUtil.h"
#include "Core/Config/MainSettings.h"
#include "Core/Config/WiimoteSettings.h"
#include "Core/ConfigManager.h"
#include "Core/Core.h"
#include "Core/IOS/IOS.h"
#include "Core/IOS/STM/STM.h"
#include "Core/State.h"
#include "Core/HW/CPU.h"
#include "Core/HW/DVD/DVDInterface.h"
#include "Core/HW/EXI/EXI_DeviceIPL.h"
#include "Core/HW/GBAPad.h"
#include "Core/HW/GCKeyboard.h"
#include "Core/HW/GCMemcard/GCMemcard.h"
#include "Core/HW/GCMemcard/GCMemcardUtils.h"
#include "Core/HW/GCPad.h"
#include "Core/HW/Memmap.h"
#include "Core/HW/ProcessorInterface.h"
#include "Core/HW/SI/SI.h"
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
    InitializeLaunchOptions(bootParams);
    initializeChannels(bootParams.instanceId, true);

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

        DolphinInputRecording MockRecording;
        MockRecording.A.push_back(ButtonRunLengthEncoded(true, 800000));
        MockRecording.AnalogStickY.push_back(AnalogRunLengthEncoded(255, 800000));
    }
    else
    {
        // Overwrite certain configs for non-mock sessions
        Config::AddLayer(GenerateInstanceConfigLoader(bootParams.instanceId));
    }

    if (recordOnLaunch)
    {
        StartRecording();
    }

    _bootToPause = bootParams.pauseOnBoot;
    SConfig::GetInstance().bBootToPause = true;
}

void Instance::SetTitle(const std::string& title)
{
}

bool Instance::Init()
{
    InitControllers();
    PrepareForTASInput();

    // Ipc post-ready callback
    _coreStateEventHandle = Core::AddOnStateChangedCallback([this](Core::State state)
    {
        if (state == Core::State::Paused)
        {
            Core::RemoveOnStateChangedCallback(&_coreStateEventHandle);
            OnCommandCompleted(DolphinInstanceIpcCall::DolphinInstance_Connect);

            if (!_bootToPause)
            {
                Core::SetState(Core::State::Running);
            }
        }
    });

    // Ipc post-connect callback
    CREATE_TO_SERVER_DATA(OnInstanceConnected, ipcData, data)
    data->_windowIdentifier = GetWindowIdentifier();
    ipcSendToServer(ipcData);

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
    // This tool is expected to be used for TASing, so we can just set this to true always
    Core::UpdateWantDeterminism(true);
    Wiimote::ResetAllWiimotes();
    Movie::SetReadOnly(false);

    for (int controllerIndex = 0; controllerIndex < SerialInterface::MAX_SI_CHANNELS; controllerIndex++)
    {
        Config::SetBaseOrCurrent(Config::GetInfoForSIDevice(static_cast<int>(controllerIndex)), SerialInterface::SIDevices::SIDEVICE_GC_CONTROLLER);
        SerialInterface::ChangeDevice(SerialInterface::SIDevices::SIDEVICE_GC_CONTROLLER, controllerIndex);
    }

    // Finalize device change
    // SerialInterface::UpdateDevices();

    if (GCAdapter::UseAdapter())
    {
        GCAdapter::StartScanThread();
    }
    else
    {
        GCAdapter::StopScanThread();
    }

    // TODO: Potentially enable this if we want to support dumping DTM files
    // Movie::BeginRecordingInput(controllers, wiimotes);*/

    Movie::SetWiiInputManip([this](WiimoteCommon::DataReportBuilder& rpt, int controllerId, int ext, const WiimoteEmu::EncryptionKey& key)
    {
    });

    Movie::SetGCInputManip([this](GCPadStatus* padStatus, int controllerId)
    {
        static const int FIRST_CONTROLLER = 0;
        static const int LAST_CONTROLLER = 3;

        if (controllerId < FIRST_CONTROLLER || controllerId > LAST_CONTROLLER)
        {
            Log(Common::Log::LogLevel::LERROR, "Unexpected controller id");
        }

        // Record or playback
        switch (_instanceState)
        {
            case RecordingState::Playback:
            {
                if (controllerId == LAST_CONTROLLER)
                {
                    CheckGcFrameAdvance(padStatus, controllerId, false);
                }

                if (!_playbackInputs[controllerId].HasNext())
                {
                    // Log(Common::Log::LogLevel::LERROR, "Unexpected end of playback input");
                    return;
                }

                DolphinControllerState padState = _playbackInputs[controllerId].PopNext();

                if ((int(padState.GameCubeEvents) & int(DolphinControllerState::GameCubeEventFlags::OpenDiscCover)) != 0)
                {
                    Core::RunAsCPUThread([=]
                    {
                        DVDInterface::EjectDisc(DVDInterface::EjectCause::User);
                    });
                }

                if ((int(padState.GameCubeEvents) & int(DolphinControllerState::GameCubeEventFlags::DiscChange)) != 0)
                {
                    Core::RunAsCPUThread([=]
                    {
                        // TODO: We need to encode the path for this somehow or another
                        DVDInterface::ChangeDisc("TODO");
                    });
                }

                if ((int(padState.GameCubeEvents) & int(DolphinControllerState::GameCubeEventFlags::ConsoleReset)) != 0)
                {
                    ProcessorInterface::ResetButton_Tap();
                }

                // Process device change requests
                if (padState.ControllerChange != DolphinControllerState::ControllerChangeEvent::None)
                {
                    SerialInterface::SIDevices newDevice;

                    switch (padState.ControllerChange)
                    {
                        default:
                        case DolphinControllerState::ControllerChangeEvent::ChangeControllerGC: newDevice = SerialInterface::SIDevices::SIDEVICE_GC_CONTROLLER; break;
                        case DolphinControllerState::ControllerChangeEvent::ChangeControllerGBA: newDevice = SerialInterface::SIDevices::SIDEVICE_GC_GBA_EMULATED; break;
                        case DolphinControllerState::ControllerChangeEvent::ChangeControllerBongos: newDevice = SerialInterface::SIDevices::SIDEVICE_DANCEMAT; break;
                        case DolphinControllerState::ControllerChangeEvent::ChangeControllerDanceMat: newDevice = SerialInterface::SIDevices::SIDEVICE_GC_TARUKONGA; break;
                        case DolphinControllerState::ControllerChangeEvent::ChangeControllerSteering: newDevice = SerialInterface::SIDevices::SIDEVICE_GC_STEERING; break;
                        case DolphinControllerState::ControllerChangeEvent::ChangeControllerNoDevice: newDevice = SerialInterface::SIDevices::SIDEVICE_NONE; break;
                    }

                    Config::SetBaseOrCurrent(Config::GetInfoForSIDevice(static_cast<int>(controllerId)), newDevice);
                    SerialInterface::ChangeDevice(newDevice, controllerId);

                    // Finalize device change
                    SerialInterface::UpdateDevices();
                }

                // Presumably it makes sense to copy the playback inputs after changing controller devices is complete
                InstanceUtils::CopyControllerStateToGcPadStatus(padState, padStatus);
                
                // Inputs complete! Ready for next command
                if (!_playbackInputs[controllerId].HasNext())
                {
                    Core::QueueHostJob([=]
                    {
                        Core::SetState(Core::State::Paused);
                        OnCommandCompleted(DolphinInstanceIpcCall::DolphinInstance_PlayInputs);
                    });
                    _instanceState = RecordingState::None;
                }
                break;
            }
            case RecordingState::Recording:
            {
                if (controllerId == LAST_CONTROLLER)
                {
                    CheckGcFrameAdvance(padStatus, controllerId, false);
                }

                if (_isRecordingController[controllerId])
                {
                    _recordingInputs[controllerId].PushNext(_hardwareInputStates[controllerId]);
                }
                break;
            }
            case RecordingState::None:
            default:
            {
                if (controllerId == LAST_CONTROLLER)
                {
                    CheckGcFrameAdvance(padStatus, controllerId, false);
                }
                break;
            }
        }
    });
}

void Instance::CheckGcFrameAdvance(GCPadStatus* padStatus, int controllerId, bool checkInputs)
{
    // Perform frame advance
    if (_framesToAdvance > 0)
    {
        if (checkInputs && !_shouldUseHardwareController)
        {
            // Replace hardware input with TAS input
            InstanceUtils::CopyControllerStateToGcPadStatus(_tasInputStates[controllerId], padStatus);
        }

        if (--_framesToAdvance <= 0)
        {
            Core::QueueHostJob([=]
            {
                Core::SetState(Core::State::Paused);
                OnCommandCompleted(DolphinInstanceIpcCall::DolphinInstance_FrameAdvance);
            });
        }
    }

    // Update stored input states, which will be applied to the game and potentially recordings (either from TAS input or hardware controller)
    if (checkInputs)
    {
        InstanceUtils::CopyGcPadStatusToControllerState(padStatus, _hardwareInputStates[controllerId]);
    }
}

INSTANCE_FUNC_BODY(Instance, Connect, params)
{
}

INSTANCE_FUNC_BODY(Instance, Heartbeat, params)
{
    _lastHeartbeat = std::chrono::system_clock::now();
    _shouldUseHardwareController = params._shouldUseHardwareController;

    // Acknowledge heartbeat, sending over any state data the server may want.
    CREATE_TO_SERVER_DATA(OnInstanceHeartbeatAcknowledged, ipcData, data)
    data->_isRecording = _instanceState == RecordingState::Recording;
    data->_isPaused = Core::GetState() == Core::State::Paused;
    data->_hardwareInputStates[0] = _hardwareInputStates[0];
    data->_hardwareInputStates[1] = _hardwareInputStates[1];
    data->_hardwareInputStates[2] = _hardwareInputStates[2];
    data->_hardwareInputStates[3] = _hardwareInputStates[3];
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

    _isRecordingController[0] = params._recordControllers[0];
    _isRecordingController[1] = params._recordControllers[1];
    _isRecordingController[2] = params._recordControllers[2];
    _isRecordingController[3] = params._recordControllers[3];

    if (params._unpauseInstance)
    {
        if (Core::GetState() == Core::State::Paused)
        {
            Core::SetState(Core::State::Running);
        }
    }

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
    _playbackInputs[0] = std::move(params._inputRecording[0]);
    _playbackInputs[1] = std::move(params._inputRecording[1]);
    _playbackInputs[2] = std::move(params._inputRecording[2]);
    _playbackInputs[3] = std::move(params._inputRecording[3]);

    if (Core::GetState() == Core::State::Paused)
    {
        Core::SetState(Core::State::Running);
    }
    
    // This checks if any controllers have inputs. HasNext() is fine to call because nothing has been popped yet.
    if (!_playbackInputs[0].HasNext()
        && !_playbackInputs[1].HasNext()
        && !_playbackInputs[2].HasNext()
        && !_playbackInputs[3].HasNext())
    {
        OnCommandCompleted(DolphinInstanceIpcCall::DolphinInstance_PlayInputs);
        return;
    }

    _instanceState = RecordingState::Playback;
}

INSTANCE_FUNC_BODY(Instance, FrameAdvance, params)
{
    _framesToAdvance = params._numFrames;

    if (Core::GetState() == Core::State::Paused)
    {
        Core::SetState(Core::State::Running);
    }
}

INSTANCE_FUNC_BODY(Instance, SetTasInput, params)
{
    _tasInputStates[0] = params._tasInputStates[0];
    _tasInputStates[1] = params._tasInputStates[1];
    _tasInputStates[2] = params._tasInputStates[2];
    _tasInputStates[3] = params._tasInputStates[3];
}

INSTANCE_FUNC_BODY(Instance, CreateSaveState, params)
{
    if (!params._filePathNoExtension.empty())
    {
        // Dump the save state
        std::string savFile = params._filePathNoExtension + ".sav";
        if (File::Exists(savFile))
        {
            File::Delete(savFile);
        }
        State::SaveAs(savFile, true);

        // Dump memory card info for this game
        if (params._saveMemoryCards)
        {
            std::string cardAFile = params._filePathNoExtension + ".cardA.gci";
            std::string cardBFile = params._filePathNoExtension + ".cardB.gci";
            InstanceUtils::ExportGci(DolphinSlot::SlotA, cardAFile);
            InstanceUtils::ExportGci(DolphinSlot::SlotB, cardBFile);
        }
    }

    CREATE_TO_SERVER_DATA(OnInstanceSaveStateCreated, ipcData, data)
    data->_filePathNoExtension = params._filePathNoExtension;
    data->_inputRecording[0] = _recordingInputs[0];
    data->_inputRecording[1] = _recordingInputs[1];
    data->_inputRecording[2] = _recordingInputs[2];
    data->_inputRecording[3] = _recordingInputs[3];
    ipcSendToServer(ipcData);

    OnCommandCompleted(DolphinInstanceIpcCall::DolphinInstance_CreateSaveState);
}

INSTANCE_FUNC_BODY(Instance, LoadSaveState, params)
{
    if (File::Exists(params._saveFilePath))
    {
        State::LoadAs(params._saveFilePath);
    }

    if (File::Exists(params._optionalMemoryCardDataAPath))
    {
        InstanceUtils::ImportGci(DolphinSlot::SlotA, params._optionalMemoryCardDataAPath);
    }

    if (File::Exists(params._optionalMemoryCardDataBPath))
    {
        InstanceUtils::ImportGci(DolphinSlot::SlotB, params._optionalMemoryCardDataBPath);
    }

    OnCommandCompleted(DolphinInstanceIpcCall::DolphinInstance_LoadSaveState);
}

INSTANCE_FUNC_BODY(Instance, LoadMemoryCardData, params)
{
    if (File::Exists(params._optionalMemoryCardDataAPath))
    {
        InstanceUtils::ImportGci(DolphinSlot::SlotA, params._optionalMemoryCardDataAPath);
    }

    if (File::Exists(params._optionalMemoryCardDataBPath))
    {
        InstanceUtils::ImportGci(DolphinSlot::SlotB, params._optionalMemoryCardDataBPath);
    }

    OnCommandCompleted(DolphinInstanceIpcCall::DolphinInstance_LoadMemoryCardData);
}

INSTANCE_FUNC_BODY(Instance, FormatMemoryCard, params)
{
    InstanceUtils::FormatMemoryCard(params._slot, params._encoding, params._cardSize);

    OnCommandCompleted(DolphinInstanceIpcCall::DolphinInstance_FormatMemoryCard);
}

INSTANCE_FUNC_BODY(Instance, ReadMemory, params)
{
    u32 address = InstanceUtils::ResolvePointer(params._address, params._pointerOffsets);

    CREATE_TO_SERVER_DATA(OnInstanceMemoryRead, ipcData, data)
    data->_bytes = InstanceUtils::ReadBytes(address, params._numberOfBytes);
    ipcSendToServer(ipcData);

    OnCommandCompleted(DolphinInstanceIpcCall::DolphinInstance_ReadMemory);
}

INSTANCE_FUNC_BODY(Instance, WriteMemory, params)
{
    u32 address = InstanceUtils::ResolvePointer(params._address, params._pointerOffsets);

    CREATE_TO_SERVER_DATA(OnInstanceMemoryWrite, ipcData, data)
    data->_success = InstanceUtils::WriteBytes(address, params._bytes);
    ipcSendToServer(ipcData);

    OnCommandCompleted(DolphinInstanceIpcCall::DolphinInstance_WriteMemory);
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
        // RequestShutdown();
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
}

void Instance::StopRecording()
{
    if (_instanceState != RecordingState::Recording)
    {
        return;
    }

    _instanceState = RecordingState::None;

    CREATE_TO_SERVER_DATA(OnInstanceRecordingStopped, ipcData, data)
    data->_inputRecording[0] = _recordingInputs[0];
    data->_inputRecording[1] = _recordingInputs[1];
    data->_inputRecording[2] = _recordingInputs[2];
    data->_inputRecording[3] = _recordingInputs[3];
    ipcSendToServer(ipcData);

    _recordingInputs[0].Clear();
    _recordingInputs[1].Clear();
    _recordingInputs[2].Clear();
    _recordingInputs[3].Clear();
}

void Instance::OnCommandCompleted(DolphinInstanceIpcCall completedCommand)
{
    CREATE_TO_SERVER_DATA(OnInstanceCommandCompleted, ipcData, data)
    data->_completedCommand = completedCommand;
    ipcSendToServer(ipcData);
}

void Instance::Log(Common::Log::LogLevel level, const char* text)
{
    // Intentionally using the same enum values so we can cast like this
    ToServerParams_OnInstanceLogOutput::LogLevel logLevel = (ToServerParams_OnInstanceLogOutput::LogLevel)level;
    std::string logString = std::string(text);

    CREATE_TO_SERVER_DATA(OnInstanceLogOutput, ipcData, data)
    data->_logLevel = logLevel;
    data->_logString = logString;
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
