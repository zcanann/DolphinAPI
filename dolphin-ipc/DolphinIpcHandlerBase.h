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
	#define INSTANCE_FUNC(Name) virtual void DolphinInstance_ ## Name(const ToInstanceParams_ ## Name& params ## Name) { NOT_IMPLEMENTED(); }
	#define INSTANCE_FUNC_OVERRIDE(Name) virtual void DolphinInstance_ ## Name(const ToInstanceParams_ ## Name& params ## Name) override;
	INSTANCE_FUNC(Connect)
	INSTANCE_FUNC(Heartbeat)
	INSTANCE_FUNC(Terminate)
	INSTANCE_FUNC(StartRecordingInput)
	INSTANCE_FUNC(StopRecordingInput)
	INSTANCE_FUNC(PauseEmulation)
	INSTANCE_FUNC(ResumeEmulation)
	INSTANCE_FUNC(PlayInputs)
	INSTANCE_FUNC(FrameAdvance)
	INSTANCE_FUNC(CreateSaveState)
	INSTANCE_FUNC(LoadSaveState)
	INSTANCE_FUNC(FormatMemoryCard)

	// Server implemented functions
protected:
	#define SERVER_FUNC(Name) virtual void DolphinServer_ ## Name(const ToServerParams_ ## Name& params ## Name) { NOT_IMPLEMENTED(); }
	#define SERVER_FUNC_OVERRIDE(Name) virtual void DolphinServer_ ## Name(const ToServerParams_ ## Name& params ## Name) override;
	SERVER_FUNC(OnInstanceConnected)
	SERVER_FUNC(OnInstanceReady)
	SERVER_FUNC(OnInstanceHeartbeatAcknowledged)
	SERVER_FUNC(OnInstanceLogOutput)
	SERVER_FUNC(OnInstanceTerminated)
	SERVER_FUNC(OnInstanceRecordingStopped)
	SERVER_FUNC(OnInstanceSaveStateCreated)
	SERVER_FUNC(OnInstanceMemoryCardFormatted)

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
