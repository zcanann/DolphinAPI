#include "DolphinIpcHandlerBase.h"

// Prevent errors in cereal that propagate to Unreal where __GNUC__ is not defined
#define __GNUC__ (false)

#include "cereal/cereal.hpp"
#include "cereal/access.hpp"
#include "cereal/types/common.hpp"
#include "cereal/types/string.hpp"
#include "cereal/types/map.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/types/memory.hpp"
#include "cereal/archives/binary.hpp"

#undef __GNUC__

const std::string DolphinIpcHandlerBase::ChannelNameInstanceToServerBase = "dol-i2s-";
const std::string DolphinIpcHandlerBase::ChannelNameServerToInstanceBase = "dol-s2i-";

DolphinIpcHandlerBase::DolphinIpcHandlerBase()
{
}

DolphinIpcHandlerBase::~DolphinIpcHandlerBase()
{
    if (_instanceToServer)
    {
        delete(_instanceToServer);
        _instanceToServer = nullptr;
    }

    if (_serverToInstance)
    {
        delete(_serverToInstance);
        _serverToInstance = nullptr;
    }
}

void DolphinIpcHandlerBase::initializeChannels(const std::string& uniqueChannelId, bool isInstance)
{
    std::string uniqueInstanceToServerChannel = ChannelNameInstanceToServerBase + uniqueChannelId;
    std::string uniqueServerToInstanceChannel = ChannelNameServerToInstanceBase + uniqueChannelId;
    _isInstance = isInstance;

    std::cout << __func__ << ": instance channel: " << uniqueInstanceToServerChannel << std::endl;
    std::cout << __func__ << ": server channel: " << uniqueServerToInstanceChannel << std::endl;

    if (_isInstance)
    {
        _instanceToServer = new ipc::channel(uniqueInstanceToServerChannel.c_str(), ipc::sender);
        _serverToInstance = new ipc::channel(uniqueServerToInstanceChannel.c_str(), ipc::receiver);
    }
    else
    {
        _instanceToServer = new ipc::channel(uniqueInstanceToServerChannel.c_str(), ipc::receiver);
        _serverToInstance = new ipc::channel(uniqueServerToInstanceChannel.c_str(), ipc::sender);
    }
}

void DolphinIpcHandlerBase::ipcSendToInstance(const DolphinIpcToInstanceData& data)
{
    ipcSendData(_serverToInstance, data);
}

void DolphinIpcHandlerBase::ipcSendToServer(const DolphinIpcToServerData& data)
{
    ipcSendData(_instanceToServer, data);
}

template<class T>
void DolphinIpcHandlerBase::ipcSendData(ipc::channel* channel, const T& data)
{
    if (channel != nullptr)
    {
        std::stringstream memoryStream(std::ios::binary | std::ios::out | std::ios::in);
        cereal::BinaryOutputArchive serializer = { memoryStream };
        serializer(data);
        std::string dataBuffer = memoryStream.str();

        std::cout << __func__ << ": try send..." << std::endl;
        if (channel->send(dataBuffer))
        {
            std::cout << __func__ << ": sent " << dataBuffer.size() << " bytes" << std::endl;
        }
    }
    else
    {
        std::cerr << __func__ << ": connect failed.\n";
    }
}

template<class T>
void DolphinIpcHandlerBase::ipcReadData(ipc::channel* channel, std::function<void(const T&)> onDeserialize)
{
    if (channel == nullptr)
    {
        return;
    }

    ipc::buff_t rawData = channel->try_recv();

    while (!rawData.empty())
    {
        size_t dataSize = rawData.size();

        std::cout << __func__ << ": recv " << dataSize << " bytes" << std::endl;

        T data;
        std::istringstream memoryStream(std::string((char*)rawData.data(), dataSize), std::ios::binary);
        cereal::BinaryInputArchive deserializer(memoryStream);
        deserializer(data);
        onDeserialize(data);

        rawData = channel->try_recv();
    }
}

void DolphinIpcHandlerBase::updateIpcListen()
{
    if (_isInstance)
    {
        ipcReadData<DolphinIpcToInstanceData>(_serverToInstance, [=](const DolphinIpcToInstanceData& data) { onServerToInstanceDataReceived(data); });
    }
    else
    {
        ipcReadData<DolphinIpcToServerData>(_instanceToServer, [=](const DolphinIpcToServerData& data) { onInstanceToServerDataReceived(data); });
    }
}

void DolphinIpcHandlerBase::onInstanceToServerDataReceived(const DolphinIpcToServerData& data)
{
    switch (data._call)
    {
        case DolphinServerIpcCall::DolphinServer_OnInstanceConnected: DolphinServer_OnInstanceConnected(*data._params._onInstanceConnectedParams); break;
        case DolphinServerIpcCall::DolphinServer_OnInstanceReady: DolphinServer_OnInstanceReady(*data._params._onInstanceReadyParams); break;
        case DolphinServerIpcCall::DolphinServer_OnInstanceHeartbeatAcknowledged: DolphinServer_OnInstanceHeartbeatAcknowledged(*data._params._onInstanceHeartbeatAcknowledged); break;
        case DolphinServerIpcCall::DolphinServer_OnInstanceTerminated: DolphinServer_OnInstanceTerminated(*data._params._onInstanceTerminatedParams); break;
        case DolphinServerIpcCall::DolphinServer_OnInstanceRecordingStopped: DolphinServer_OnInstanceRecordingStopped(*data._params._onInstanceRecordingStopped); break;
        case DolphinServerIpcCall::DolphinServer_OnInstanceSaveStateCreated: DolphinServer_OnInstanceSaveStateCreated(*data._params._onInstanceSaveStateCreated); break;
        case DolphinServerIpcCall::Null: default: std::cout << "NULL instance => server call!" << std::endl; break;
    }
}

void DolphinIpcHandlerBase::onServerToInstanceDataReceived(const DolphinIpcToInstanceData& data)
{
    switch (data._call)
    {
        case DolphinInstanceIpcCall::DolphinInstance_Connect: DolphinInstance_Connect(*data._params._connectParams); break;
        case DolphinInstanceIpcCall::DolphinInstance_Heartbeat: DolphinInstance_Heartbeat(*data._params._heartbeatParams); break;
        case DolphinInstanceIpcCall::DolphinInstance_Terminate: DolphinInstance_Terminate(*data._params._terminateParams); break;
        case DolphinInstanceIpcCall::DolphinInstance_StartRecordingInput: DolphinInstance_StartRecordingInput(*data._params._startRecordingInputParams); break;
        case DolphinInstanceIpcCall::DolphinInstance_StopRecordingInput: DolphinInstance_StopRecordingInput(*data._params._stopRecordingInputParams); break;
        case DolphinInstanceIpcCall::DolphinInstance_PauseEmulation: DolphinInstance_PauseEmulation(*data._params._pauseEmulationParams); break;
        case DolphinInstanceIpcCall::DolphinInstance_ResumeEmulation: DolphinInstance_ResumeEmulation(*data._params._resumeEmulationParams); break;
        case DolphinInstanceIpcCall::DolphinInstance_PlayInputs: DolphinInstance_PlayInputs(*data._params._playInputsParams); break;
        case DolphinInstanceIpcCall::DolphinInstance_FrameAdvance: DolphinInstance_FrameAdvance(*data._params._frameAdvanceParams); break;
        case DolphinInstanceIpcCall::DolphinInstance_CreateSaveState: DolphinInstance_CreateSaveState(*data._params._createSaveStateParams); break;
        case DolphinInstanceIpcCall::DolphinInstance_LoadSaveState: DolphinInstance_LoadSaveState(*data._params._loadSaveStateParams); break;
        case DolphinInstanceIpcCall::Null: default: std::cout << "NULL server => instance call!" << std::endl; break;
    }
}
