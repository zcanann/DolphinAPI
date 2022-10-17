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

#include "Ipc/NamedPipe.h"

const std::string DolphinIpcHandlerBase::ChannelNameInstanceToServerBase = "dol-i2s-";
const std::string DolphinIpcHandlerBase::ChannelNameServerToInstanceBase = "dol-s2i-";

DolphinIpcHandlerBase::DolphinIpcHandlerBase()
{
}

DolphinIpcHandlerBase::~DolphinIpcHandlerBase()
{
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
        _instanceToServer = std::make_shared<NamedPipe>(uniqueInstanceToServerChannel, false);
        _serverToInstance = std::make_shared<NamedPipe>(uniqueServerToInstanceChannel, false);
    }
    else
    {
        _serverToInstance = std::make_shared<NamedPipe>(uniqueServerToInstanceChannel, true);
        _instanceToServer = std::make_shared<NamedPipe>(uniqueInstanceToServerChannel, true);
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
void DolphinIpcHandlerBase::ipcSendData(std::shared_ptr<NamedPipe>& channel, const T& data)
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
void DolphinIpcHandlerBase::ipcReadData(std::shared_ptr<NamedPipe>& channel, std::function<void(const T&)> onDeserialize)
{
    if (channel == nullptr)
    {
        return;
    }

    std::string rawData;

    while (channel->recv(rawData))
    {
        std::cout << __func__ << ": recv " << rawData.size() << " bytes" << std::endl;

        T data;
        std::stringstream memoryStream(rawData, std::ios::binary | std::ios::out | std::ios::in);
        cereal::BinaryInputArchive deserializer(memoryStream);
        deserializer(data);
        onDeserialize(data);
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

#define SERVER_DISPATCH(Name) case DolphinServerIpcCall::DolphinServer_ ## Name: DolphinServer_ ## Name(*data._params._params ## Name); break;
void DolphinIpcHandlerBase::onInstanceToServerDataReceived(const DolphinIpcToServerData& data)
{
    switch (data._call)
    {
        SERVER_DISPATCH(OnInstanceConnected)
        SERVER_DISPATCH(OnInstanceCommandCompleted)
        SERVER_DISPATCH(OnInstanceHeartbeatAcknowledged)
        SERVER_DISPATCH(OnInstanceLogOutput)
        SERVER_DISPATCH(OnInstanceTerminated)
        SERVER_DISPATCH(OnInstanceRecordingStopped)
        SERVER_DISPATCH(OnInstanceSaveStateCreated)
        SERVER_DISPATCH(OnInstanceMemoryCardFormatted)
        SERVER_DISPATCH(OnInstanceMemoryRead)
        SERVER_DISPATCH(OnInstanceMemoryWrite)
        SERVER_DISPATCH(OnInstanceRenderGba)
        case DolphinServerIpcCall::Null: default: std::cout << "NULL instance => server call!" << std::endl; break;
    }
}

#define INSTANCE_DISPATCH(Name) case DolphinInstanceIpcCall::DolphinInstance_ ## Name: DolphinInstance_ ## Name(*data._params._params ## Name); break;
void DolphinIpcHandlerBase::onServerToInstanceDataReceived(const DolphinIpcToInstanceData& data)
{
    switch (data._call)
    {
        INSTANCE_DISPATCH(Connect)
        INSTANCE_DISPATCH(Heartbeat)
        INSTANCE_DISPATCH(Terminate)
        INSTANCE_DISPATCH(StartRecordingInput)
        INSTANCE_DISPATCH(StopRecordingInput)
        INSTANCE_DISPATCH(PauseEmulation)
        INSTANCE_DISPATCH(ResumeEmulation)
        INSTANCE_DISPATCH(PlayInputs)
        INSTANCE_DISPATCH(FrameAdvance)
        INSTANCE_DISPATCH(SetTasInput)
        INSTANCE_DISPATCH(CreateSaveState)
        INSTANCE_DISPATCH(LoadSaveState)
        INSTANCE_DISPATCH(FormatMemoryCard)
        INSTANCE_DISPATCH(ReadMemory)
        INSTANCE_DISPATCH(WriteMemory)
        case DolphinInstanceIpcCall::Null: default: std::cout << "NULL server => instance call!" << std::endl; break;
    }
}
