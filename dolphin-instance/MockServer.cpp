
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

SERVER_FUNC_BODY(MockServer, OnInstanceCommandCompleted, params)
{
    std::cout << "recieved instance command completed" << std::endl;
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
    std::cout << "recieved inputs - controller 0 " << params._inputRecording[0].Size() << std::endl;
    std::cout << "recieved inputs - controller 1 " << params._inputRecording[1].Size() << std::endl;
    std::cout << "recieved inputs - controller 2 " << params._inputRecording[2].Size() << std::endl;
    std::cout << "recieved inputs - controller 3 " << params._inputRecording[3].Size() << std::endl;
}

SERVER_FUNC_BODY(MockServer, OnInstanceSaveStateCreated, params)
{
    std::cout << "save state created " << params._filePathNoExtension << std::endl;
}

SERVER_FUNC_BODY(MockServer, OnInstanceRenderGba, params)
{
}
