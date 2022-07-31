
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

void MockServer::DolphinServer_OnInstanceConnected(const ToServerParams_OnInstanceConnected& OnInstanceConnectedParams)
{
    std::cout << "recieved instance connected" << std::endl;
}

void MockServer::DolphinServer_OnInstanceHeartbeatAcknowledged(const ToServerParams_OnInstanceHeartbeatAcknowledged& onInstanceHeartbeatAcknowledgedParams)
{
}

void MockServer::DolphinServer_OnInstanceTerminated(const ToServerParams_OnInstanceTerminated& OnInstanceTerminatedParams)
{
    std::cout << "recieved instance terminated" << std::endl;
}

void MockServer::DolphinServer_OnInstanceRecordingStopped(const ToServerParams_OnInstanceRecordingStopped& onInstanceRecordingStopped)
{
    std::cout << "recieved " << onInstanceRecordingStopped._inputStates.size() << std::endl;
}

void MockServer::DolphinServer_OnInstanceSaveStateCreated(const ToServerParams_OnInstanceSaveStateCreated& onInstanceSaveStateCreated)
{
    std::cout << "save state created " << onInstanceSaveStateCreated._filePath << std::endl;
}
