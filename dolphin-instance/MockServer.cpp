
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

SERVER_FUNC_BODY(MockServer, OnInstanceConnected, params)
{
    std::cout << "recieved instance connected" << std::endl;
}

SERVER_FUNC_BODY(MockServer, OnInstanceReady, params)
{
    std::cout << "recieved instance ready" << std::endl;
}

SERVER_FUNC_BODY(MockServer, OnInstanceHeartbeatAcknowledged, params)
{
    std::cout << "recieved instance ready" << std::endl;
}

SERVER_FUNC_BODY(MockServer, OnInstanceLogOutput, params)
{
    std::cout << "[LOG] " << params._logString << std::endl;
}

SERVER_FUNC_BODY(MockServer, OnInstanceTerminated, params)
{
    std::cout << "recieved instance terminated" << std::endl;
}

SERVER_FUNC_BODY(MockServer, OnInstanceRecordingStopped, params)
{
    std::cout << "recieved " << params._inputStates.size() << std::endl;
}

SERVER_FUNC_BODY(MockServer, OnInstanceSaveStateCreated, params)
{
    std::cout << "save state created " << params._filePath << std::endl;
}

SERVER_FUNC_BODY(MockServer, OnInstanceMemoryCardFormatted, params)
{
    std::cout << "memory card formatted" << std::endl;
}
