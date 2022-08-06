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
#define CREATE_TO_INSTANCE_DATA(IpcCall, IpcVariableName, VariableName) \
	DolphinIpcToInstanceData IpcVariableName; \
	std::shared_ptr<ToInstanceParams_ ## IpcCall> VariableName = std::make_shared<ToInstanceParams_ ## IpcCall>(); \
	IpcVariableName._call = DolphinInstanceIpcCall::DolphinInstance_ ## IpcCall; \
	IpcVariableName._params._params ## IpcCall = VariableName;
#define CREATE_TO_SERVER_DATA(IpcCall, IpcVariableName, VariableName) \
	DolphinIpcToServerData IpcVariableName; \
	std::shared_ptr<ToServerParams_ ## IpcCall> VariableName = std::make_shared<ToServerParams_ ## IpcCall>(); \
	IpcVariableName._call = DolphinServerIpcCall::DolphinServer_ ## IpcCall; \
	IpcVariableName._params._params ## IpcCall = VariableName;

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
	#define INSTANCE_FUNC_BODY(Class, Name, params) void Class::DolphinInstance_ ## Name(const ToInstanceParams_ ## Name& params)
	INSTANCE_FUNC(Connect)
	INSTANCE_FUNC(Heartbeat)
	INSTANCE_FUNC(Terminate)
	INSTANCE_FUNC(StartRecordingInput)
	INSTANCE_FUNC(StopRecordingInput)
	INSTANCE_FUNC(PauseEmulation)
	INSTANCE_FUNC(ResumeEmulation)
	INSTANCE_FUNC(PlayInputs)
	INSTANCE_FUNC(FrameAdvance)
	INSTANCE_FUNC(FrameAdvanceWithInput)
	INSTANCE_FUNC(CreateSaveState)
	INSTANCE_FUNC(LoadSaveState)
	INSTANCE_FUNC(FormatMemoryCard)
	INSTANCE_FUNC(ReadMemory)
	INSTANCE_FUNC(WriteMemory)

	// Server implemented functions
protected:
	#define SERVER_FUNC(Name) virtual void DolphinServer_ ## Name(const ToServerParams_ ## Name& params ## Name) { NOT_IMPLEMENTED(); }
	#define SERVER_FUNC_OVERRIDE(Name) virtual void DolphinServer_ ## Name(const ToServerParams_ ## Name& params ## Name) override;
	#define SERVER_FUNC_BODY(Class, Name, params) void Class::DolphinServer_ ## Name(const ToServerParams_ ## Name& params)
	SERVER_FUNC(OnInstanceConnected)
	SERVER_FUNC(OnInstanceCommandCompleted)
	SERVER_FUNC(OnInstanceHeartbeatAcknowledged)
	SERVER_FUNC(OnInstanceLogOutput)
	SERVER_FUNC(OnInstanceTerminated)
	SERVER_FUNC(OnInstanceRecordingStopped)
	SERVER_FUNC(OnInstanceSaveStateCreated)
	SERVER_FUNC(OnInstanceMemoryCardFormatted)
	SERVER_FUNC(OnInstanceMemoryRead)
	SERVER_FUNC(OnInstanceMemoryWrite)

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
