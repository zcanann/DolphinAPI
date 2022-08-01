
#include "MockServer.h"

MockServer::MockServer(const std::string& instanceId)
{
    initializeChannels(instanceId, false);
}

MockServer::~MockServer()
{
}

void MockServer::Update()
{
    updateIpcListen();

    // Send a heartbeat to the running instance
    DolphinIpcToInstanceData ipcData;
    std::shared_ptr<ToInstanceParams_Heartbeat> data = std::make_shared<ToInstanceParams_Heartbeat>();
    ipcData._call = DolphinInstanceIpcCall::DolphinInstance_Heartbeat;
    ipcSendToInstance(ipcData);
}

void MockServer::DolphinServer_OnInstanceConnected(const ToServerParams_OnInstanceConnected& params)
{
    std::cout << "recieved instance connected" << std::endl;
}

void MockServer::DolphinServer_OnInstanceReady(const ToServerParams_OnInstanceReady& params)
{
    std::cout << "recieved instance ready" << std::endl;
}

void MockServer::DolphinServer_OnInstanceHeartbeatAcknowledged(const ToServerParams_OnInstanceHeartbeatAcknowledged& params)
{
}

void MockServer::DolphinServer_OnInstanceLogOutput(const ToServerParams_OnInstanceLogOutput& params)
{
    std::cout << "[LOG] " << params._logString << std::endl;
}

void MockServer::DolphinServer_OnInstanceTerminated(const ToServerParams_OnInstanceTerminated& params)
{
    std::cout << "recieved instance terminated" << std::endl;
}

void MockServer::DolphinServer_OnInstanceRecordingStopped(const ToServerParams_OnInstanceRecordingStopped& params)
{
    std::cout << "recieved " << params._inputStates.size() << std::endl;
}

void MockServer::DolphinServer_OnInstanceSaveStateCreated(const ToServerParams_OnInstanceSaveStateCreated& params)
{
    std::cout << "save state created " << params._filePath << std::endl;
}

void MockServer::DolphinServer_OnInstanceMemoryCardFormatted(const ToServerParams_OnInstanceMemoryCardFormatted& params)
{
    std::cout << "memory card formatted" << std::endl;
}
