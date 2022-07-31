// Copyright 2022 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Instance.h"

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
#include "Core/HW/GBAPad.h"
#include "Core/HW/GCKeyboard.h"
#include "Core/HW/GCPad.h"
#include "Core/HW/ProcessorInterface.h"
#include "Core/HW/SI/SI_Device.h"
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
    ipcData._params._onInstanceConnectedParams = data;
    ipcSendToServer(ipcData);

    // Ipc post-ready callback
    _coreStateEventHandle = Core::AddOnStateChangedCallback([this](Core::State state)
    {
        if (state == Core::State::Running || state == Core::State::Paused)
        {
            OnReadyForNextCommand();
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

    Movie::SetGCInputManip([this](GCPadStatus* PadStatus, int ControllerId)
    {
        switch (_instanceState)
        {
            case RecordingState::Playback:
            {
                if (PadStatus == nullptr || _playbackInputs.size() <= 0)
                {
                    return;
                }

                DolphinControllerState padState = _playbackInputs.front();
                _playbackInputs.erase(_playbackInputs.begin());

                PadStatus->isConnected = padState.IsConnected;

                PadStatus->triggerLeft = padState.TriggerL;
                PadStatus->triggerRight = padState.TriggerR;

                PadStatus->stickX = padState.AnalogStickX;
                PadStatus->stickY = padState.AnalogStickY;

                PadStatus->substickX = padState.CStickX;
                PadStatus->substickY = padState.CStickY;

                PadStatus->button = 0;
                PadStatus->button |= PAD_USE_ORIGIN;
                PadStatus->button |= padState.A ? PAD_BUTTON_A : 0;
                PadStatus->analogA = padState.A ? 0xFF : 0x00;
                PadStatus->button |= padState.B ? PAD_BUTTON_B : 0;
                PadStatus->analogB = padState.B ? 0xFF : 0x00;
                PadStatus->button |= padState.X ? PAD_BUTTON_X : 0;
                PadStatus->button |= padState.Y ? PAD_BUTTON_Y : 0;
                PadStatus->button |= padState.Z ? PAD_TRIGGER_Z : 0;
                PadStatus->button |= padState.Start ? PAD_BUTTON_START : 0;
                PadStatus->button |= padState.DPadUp ? PAD_BUTTON_UP : 0;
                PadStatus->button |= padState.DPadDown ? PAD_BUTTON_DOWN : 0;
                PadStatus->button |= padState.DPadLeft ? PAD_BUTTON_LEFT : 0;
                PadStatus->button |= padState.DPadRight ? PAD_BUTTON_RIGHT : 0;
                PadStatus->button |= padState.L ? PAD_TRIGGER_L : 0;
                PadStatus->button |= padState.R ? PAD_TRIGGER_R : 0;
                PadStatus->button |= padState.GetOrigin ? PAD_GET_ORIGIN : 0;

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
                    OnReadyForNextCommand();
                }
                break;
            }
            case RecordingState::Recording:
            {
                if (PadStatus == nullptr)
                {
                    return;
                }

                DolphinControllerState padState;

                padState.A = ((PadStatus->button & PAD_BUTTON_A) != 0);
                padState.B = ((PadStatus->button & PAD_BUTTON_B) != 0);
                padState.X = ((PadStatus->button & PAD_BUTTON_X) != 0);
                padState.Y = ((PadStatus->button & PAD_BUTTON_Y) != 0);
                padState.Z = ((PadStatus->button & PAD_TRIGGER_Z) != 0);
                padState.Start = ((PadStatus->button & PAD_BUTTON_START) != 0);

                padState.DPadUp = ((PadStatus->button & PAD_BUTTON_UP) != 0);
                padState.DPadDown = ((PadStatus->button & PAD_BUTTON_DOWN) != 0);
                padState.DPadLeft = ((PadStatus->button & PAD_BUTTON_LEFT) != 0);
                padState.DPadRight = ((PadStatus->button & PAD_BUTTON_RIGHT) != 0);

                padState.L = ((PadStatus->button & PAD_TRIGGER_L) != 0);
                padState.R = ((PadStatus->button & PAD_TRIGGER_R) != 0);
                padState.TriggerL = PadStatus->triggerLeft;
                padState.TriggerR = PadStatus->triggerRight;

                padState.AnalogStickX = PadStatus->stickX;
                padState.AnalogStickY = PadStatus->stickY;

                padState.CStickX = PadStatus->substickX;
                padState.CStickY = PadStatus->substickY;

                padState.IsConnected = PadStatus->isConnected;

                padState.GetOrigin = (PadStatus->button & PAD_GET_ORIGIN) != 0;

                padState.Disc = false; // TODO
                padState.Reset = false; // TODO

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

void Instance::DolphinInstance_Connect(const ToInstanceParams_Connect& connectParams)
{
}

void Instance::DolphinInstance_Heartbeat(const ToInstanceParams_Heartbeat& heartbeatParams)
{
    _lastHeartbeat = std::chrono::system_clock::now();

    // Acknowledge heartbeat, sending over any state data the server may want.
    DolphinIpcToServerData ipcData;
    std::shared_ptr<ToServerParams_OnInstanceHeartbeatAcknowledged> data = std::make_shared<ToServerParams_OnInstanceHeartbeatAcknowledged>();
    data->_isRecording = _instanceState == RecordingState::Recording;
    data->_isPaused = Core::GetState() == Core::State::Paused;
    ipcData._call = DolphinServerIpcCall::DolphinServer_OnInstanceHeartbeatAcknowledged;
    ipcData._params._onInstanceHeartbeatAcknowledged = data;
    ipcSendToServer(ipcData);
}

void Instance::DolphinInstance_Terminate(const ToInstanceParams_Terminate& terminateParams)
{
    StopRecording();
    RequestShutdown();
}

void Instance::DolphinInstance_StartRecordingInput(const ToInstanceParams_StartRecordingInput& beginRecordingInputParams)
{
    StopRecording();
    StartRecording();
    OnReadyForNextCommand();
}

void Instance::DolphinInstance_StopRecordingInput(const ToInstanceParams_StopRecordingInput& stopRecordingInputParams)
{
    StopRecording();
    OnReadyForNextCommand();
}

void Instance::DolphinInstance_PauseEmulation(const ToInstanceParams_PauseEmulation& pauseEmulationParams)
{
    if (Core::GetState() == Core::State::Running)
    {
        Core::SetState(Core::State::Paused);
    }

    OnReadyForNextCommand();
}

void Instance::DolphinInstance_ResumeEmulation(const ToInstanceParams_ResumeEmulation& resumeEmulationParams)
{
    if (Core::GetState() == Core::State::Paused)
    {
        Core::SetState(Core::State::Running);
    }

    OnReadyForNextCommand();
}

void Instance::DolphinInstance_PlayInputs(const ToInstanceParams_PlayInputs& playInputsParams)
{
    // These vectors can be masive, use std::move to avoid an extra alloc (should be safe since _inputStates is not used after this)
    _playbackInputs = std::move(playInputsParams._inputStates);

    if (_playbackInputs.size() > 0)
    {
        _instanceState = RecordingState::Playback;
        // Send the initial flag to force determinism, since internally this checks Movie class flags which this class does not set
        Core::UpdateWantDeterminism(true);
    }

    if (Core::GetState() == Core::State::Paused)
    {
        Core::SetState(Core::State::Running);
    }
}

void Instance::DolphinInstance_FrameAdvance(const ToInstanceParams_FrameAdvance& frameAdvanceParams)
{
    Core::DoFrameStep();
    OnReadyForNextCommand();
}

void Instance::DolphinInstance_CreateSaveState(const ToInstanceParams_CreateSaveState& createSaveStateParams)
{
    if (!createSaveStateParams._filePath.empty())
    {
        if (File::Exists(createSaveStateParams._filePath))
        {
            File::Delete(createSaveStateParams._filePath);
        }

        State::SaveAs(createSaveStateParams._filePath, true);
    }
    
    DolphinIpcToServerData ipcData;
    std::shared_ptr<ToServerParams_OnInstanceSaveStateCreated> data = std::make_shared<ToServerParams_OnInstanceSaveStateCreated>();
    data->_filePath = createSaveStateParams._filePath;
    ipcData._call = DolphinServerIpcCall::DolphinServer_OnInstanceSaveStateCreated;
    ipcData._params._onInstanceSaveStateCreated = data;
    ipcSendToServer(ipcData);

    OnReadyForNextCommand();
}

void Instance::DolphinInstance_LoadSaveState(const ToInstanceParams_LoadSaveState& loadSaveStateParams)
{
    if (File::Exists(loadSaveStateParams._filePath))
    {
        State::LoadAs(loadSaveStateParams._filePath);
    }

    OnReadyForNextCommand();
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
    ipcData._params._onInstanceRecordingStopped = data;
    ipcSendToServer(ipcData);

    _recordingInputs.clear();
}

void Instance::OnReadyForNextCommand()
{
    DolphinIpcToServerData ipcData;
    std::shared_ptr<ToServerParams_OnInstanceReady> data = std::make_shared<ToServerParams_OnInstanceReady>();
    ipcData._call = DolphinServerIpcCall::DolphinServer_OnInstanceReady;
    ipcData._params._onInstanceReadyParams = data;
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
