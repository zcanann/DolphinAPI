#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif // !WIN32_LEAN_AND_MEAN

#include "DolphinIpcToInstanceData.h"
#include "DolphinIpcToServerData.h"

#include <atomic>
#include <iostream>
#include <chrono>
#include <cstddef>
#include <signal.h>
#include <streambuf>
#include <string>
#include <thread>

#include "libipc/ipc.h"

#define NOT_IMPLEMENTED() std::cout << "CALLED UNIMPLEMENTED HANDLER FUNC" << std::endl;

class DolphinIpcHandlerBase
{
public:
	DolphinIpcHandlerBase();
	virtual ~DolphinIpcHandlerBase();

	void initializeChannels(const std::string& uniqueChannelId, bool isInstance);

	void updateIpcListen();
	void ipcSendToServer(const DolphinIpcToServerData& data);
	void ipcSendToInstance(const DolphinIpcToInstanceData& data);

	// Instance implemented functions
protected:
	virtual void DolphinInstance_Connect(const ToInstanceParams_Connect& connectParams) { NOT_IMPLEMENTED(); }
	virtual void DolphinInstance_Heartbeat(const ToInstanceParams_Heartbeat& heartbeatParams) { NOT_IMPLEMENTED(); }
	virtual void DolphinInstance_Terminate(const ToInstanceParams_Terminate& terminateParams) { NOT_IMPLEMENTED(); }
	virtual void DolphinInstance_StartRecordingInput(const ToInstanceParams_StartRecordingInput& startRecordingInputParams) { NOT_IMPLEMENTED(); }
	virtual void DolphinInstance_StopRecordingInput(const ToInstanceParams_StopRecordingInput& stopRecordingInputParams) { NOT_IMPLEMENTED(); }
	virtual void DolphinInstance_PauseEmulation(const ToInstanceParams_PauseEmulation& pauseEmulationParams) { NOT_IMPLEMENTED(); }
	virtual void DolphinInstance_ResumeEmulation(const ToInstanceParams_ResumeEmulation& resumeEmulationParams) { NOT_IMPLEMENTED(); }
	virtual void DolphinInstance_PlayInputs(const ToInstanceParams_PlayInputs& playInputsParams) { NOT_IMPLEMENTED(); }
	virtual void DolphinInstance_FrameAdvance(const ToInstanceParams_FrameAdvance& frameAdvanceParams) { NOT_IMPLEMENTED(); }
	virtual void DolphinInstance_CreateSaveState(const ToInstanceParams_CreateSaveState& createSaveStateParams) { NOT_IMPLEMENTED(); }
	virtual void DolphinInstance_LoadSaveState(const ToInstanceParams_LoadSaveState& loadSaveStateParams) { NOT_IMPLEMENTED(); }

	// Server implemented functions
protected:
	virtual void DolphinServer_OnInstanceConnected(const ToServerParams_OnInstanceConnected& onInstanceConnectedParams) { NOT_IMPLEMENTED(); }
	virtual void DolphinServer_OnInstanceReady(const ToServerParams_OnInstanceReady& onInstanceReadyParams) { NOT_IMPLEMENTED(); }
	virtual void DolphinServer_OnInstanceHeartbeatAcknowledged(const ToServerParams_OnInstanceHeartbeatAcknowledged& onInstanceHeartbeatAcknowledgedParams) { NOT_IMPLEMENTED(); }
	virtual void DolphinServer_OnInstanceLogOutput(const ToServerParams_OnInstanceLogOutput& onInstanceLogOutputParams) { NOT_IMPLEMENTED(); }
	virtual void DolphinServer_OnInstanceTerminated(const ToServerParams_OnInstanceTerminated& onInstanceTerminatedParams) { NOT_IMPLEMENTED(); }
	virtual void DolphinServer_OnInstanceRecordingStopped(const ToServerParams_OnInstanceRecordingStopped& onInstanceRecordingStopped) { NOT_IMPLEMENTED(); }
	virtual void DolphinServer_OnInstanceSaveStateCreated(const ToServerParams_OnInstanceSaveStateCreated& onInstanceSaveStateCreated) { NOT_IMPLEMENTED(); }

private:
	template<class T>
	void ipcSendData(ipc::channel* channel, const T& params);

	template<class T>
	void ipcReadData(ipc::channel* channel, std::function<void(const T&)> onDeserialize);

	void onInstanceToServerDataReceived(const DolphinIpcToServerData& data);
	void onServerToInstanceDataReceived(const DolphinIpcToInstanceData& data);

	bool _isInstance = true;
	ipc::channel* _instanceToServer = nullptr;
	ipc::channel* _serverToInstance = nullptr;

	static const std::string ChannelNameInstanceToServerBase;
	static const std::string ChannelNameServerToInstanceBase;
};
