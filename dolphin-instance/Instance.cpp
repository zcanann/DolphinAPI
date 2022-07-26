// Copyright 2022 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Instance.h"

#include "InstanceConfigLoader.h"

// Dolphin includes
#include "Core/Config/MainSettings.h"
#include "Core/Config/WiimoteSettings.h"
#include "Core/ConfigManager.h"
#include "Core/Core.h"
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

namespace ProcessorInterface
{
    void PowerButton_Tap();
}

Instance::Instance(const std::string& instanceId)
{
    initializeChannels(instanceId, true);
}

Instance::~Instance()
{
}

void Instance::SetTitle(const std::string& title)
{
}

bool Instance::Init()
{
    // Ipc post-connect callback
    DolphinIpcToServerData ipcData;
    std::shared_ptr<ToServerParams_OnInstanceConnected> data = std::make_shared<ToServerParams_OnInstanceConnected>();
    data->_params = "ipc_connect_debug";
    ipcData._call = DolphinServerIpcCall::DolphinServer_OnInstanceConnected;
    ipcData._params._onInstanceConnectedParams = data;
    ipcSendToServer(ipcData);

    InitControllers();
    Config::AddLayer(GenerateInstanceConfigLoader());
    PrepareForTASInput();

    // Initialize to paused, since we will need to wait for IPC commands to send input data
    Core::SetState(Core::State::Paused);

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
    // Core::SetState(Core::State::Paused);

    Wiimote::ResetAllWiimotes();
    Core::UpdateWantDeterminism();
    Movie::SetReadOnly(false);

    Movie::SetGCInputManip([this](GCPadStatus* PadStatus, int ControllerId)
    {
        u64 frame = Movie::GetCurrentFrame();

        if (PadStatus)
        {
            if (_isRecording)
            {
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

                _inputs.push_back(padState);
            }
        }
        static int debug = 5;
        if (PadStatus)
        {
            // pad_status->triggerRight = 255;
            // pad_status->button |= PadButton::PAD_TRIGGER_R;

            /*
            if (debug-- < 0)
            {
                pad_status->button |= PadButton::PAD_BUTTON_A;
                pad_status->button |= PadButton::PAD_BUTTON_START;
                debug = 5;
            }*/
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

void Instance::DolphinInstance_BeginRecordingInput(const ToInstanceParams_BeginRecordingInput& beginRecordingInputParams)
{
    if (Core::GetState() == Core::State::Paused)
    {
        Core::SetState(Core::State::Running);
    }

    _isRecording = true;
}

void Instance::DolphinInstance_StopRecordingInput(const ToInstanceParams_StopRecordingInput& stopRecordingInputParams)
{
    _isRecording = false;

    DolphinIpcToServerData ipcData;
    std::shared_ptr<ToServerParams_OnInstanceRecordingStopped> data = std::make_shared<ToServerParams_OnInstanceRecordingStopped>();
    data->_inputStates = _inputs;
    ipcData._call = DolphinServerIpcCall::DolphinServer_OnInstanceRecordingStopped;
    ipcData._params._onInstanceRecordingStopped = data;
    ipcSendToServer(ipcData);

    _inputs.clear();
}

void Instance::UpdateRunningFlag()
{
    updateIpcListen();

    if (_shutdown_requested.TestAndClear())
    {
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

void Instance::Stop()
{
    _running.Clear();
}

void Instance::RequestShutdown()
{
    _shutdown_requested.Set();
}
