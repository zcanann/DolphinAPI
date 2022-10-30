#pragma once
// Defines the IPC callbacks that are called from an individual Dolphin instance (client) to the controlling library (server)

#include "IpcStructs.h"

// Prevent errors in cereal that propagate to Unreal where __GNUC__ is not defined
#define __GNUC__ (false)
#include "external/cereal/cereal.hpp"
#undef __GNUC__

#include <string>

enum class DolphinServerIpcCall
{
	Null,
	DolphinServer_OnInstanceConnected,
	DolphinServer_OnInstanceCommandCompleted,
	DolphinServer_OnInstanceHeartbeatAcknowledged,
	DolphinServer_OnInstanceLogOutput,
	DolphinServer_OnInstanceTerminated,
	DolphinServer_OnInstanceRecordingStopped,
	DolphinServer_OnInstanceSaveStateCreated,
	DolphinServer_OnInstanceMemoryRead,
	DolphinServer_OnInstanceMemoryWrite,
	DolphinServer_OnInstanceRenderGba,
};

struct ToServerParams_OnInstanceConnected
{
	unsigned long long _windowIdentifier = 0;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_windowIdentifier);
	}
};

struct ToServerParams_OnInstanceCommandCompleted
{
	DolphinInstanceIpcCall _completedCommand;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_completedCommand);
	}
};

struct ToServerParams_OnInstanceHeartbeatAcknowledged
{
	bool _isRecording = false;
	bool _isPaused = false;
	DolphinControllerState _hardwareInputStates[4];

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_isRecording);
		ar(_isPaused);
		ar(_hardwareInputStates[0]);
		ar(_hardwareInputStates[1]);
		ar(_hardwareInputStates[2]);
		ar(_hardwareInputStates[3]);
	}
};

struct ToServerParams_OnInstanceLogOutput
{
	enum class LogLevel
	{
		Notice = 1,
		Error = 2,
		Warning = 3,
		Info = 4,
		Debug = 5,
	};

	LogLevel _logLevel;
	std::string _logString;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_logLevel);
		ar(_logString);
	}
};

struct ToServerParams_OnInstanceTerminated
{
	template <class Archive>
	void serialize(Archive& ar)
	{
	}
};

struct ToServerParams_OnInstanceRecordingStopped
{
	DolphinInputRecording _inputRecording[4];

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_inputRecording[0]);
		ar(_inputRecording[1]);
		ar(_inputRecording[2]);
		ar(_inputRecording[3]);
	}
};

struct ToServerParams_OnInstanceSaveStateCreated
{
	std::string _filePathNoExtension;
	DolphinInputRecording _inputRecording[4];

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_filePathNoExtension);
		ar(_inputRecording[0]);
		ar(_inputRecording[1]);
		ar(_inputRecording[2]);
		ar(_inputRecording[3]);
	}
};

struct ToServerParams_OnInstanceMemoryRead
{
	std::vector<unsigned char> _bytes;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_bytes);
	}
};

struct ToServerParams_OnInstanceMemoryWrite
{
	bool _success = false;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_success);
	}
};

struct ToServerParams_OnInstanceRenderGba
{
	int _controllerIndex = 0;
	int _width = 0;
	int _height = 0;
	bool _compressed = false;
	std::vector<unsigned int> _frameBuffer;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_controllerIndex);
		ar(_width);
		ar(_height);
		ar(_compressed);
		ar(_frameBuffer);
	}
};

#define TO_SERVER_MEMBER(Name) std::shared_ptr<ToServerParams_##Name> _params ## Name;
struct DolphinIpcToServerDataParams
{
	DolphinIpcToServerDataParams() {}
	~DolphinIpcToServerDataParams() {}

	TO_SERVER_MEMBER(OnInstanceConnected)
	TO_SERVER_MEMBER(OnInstanceCommandCompleted)
	TO_SERVER_MEMBER(OnInstanceHeartbeatAcknowledged)
	TO_SERVER_MEMBER(OnInstanceLogOutput)
	TO_SERVER_MEMBER(OnInstanceTerminated)
	TO_SERVER_MEMBER(OnInstanceRecordingStopped)
	TO_SERVER_MEMBER(OnInstanceSaveStateCreated)
	TO_SERVER_MEMBER(OnInstanceMemoryRead)
	TO_SERVER_MEMBER(OnInstanceMemoryWrite)
	TO_SERVER_MEMBER(OnInstanceRenderGba)
};

#define TO_SERVER_ARCHIVE(Name) case DolphinServerIpcCall::DolphinServer_ ## Name: \
	{ \
		if (!_params._params ## Name) \
		_params._params ## Name = std::make_shared<ToServerParams_##Name>(); \
		ar(*(_params._params ## Name)); \
		break; \
	}

struct DolphinIpcToServerData
{
	DolphinServerIpcCall _call = DolphinServerIpcCall::Null;
	DolphinIpcToServerDataParams _params;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_call);
		
		switch (_call)
		{
			TO_SERVER_ARCHIVE(OnInstanceConnected)
			TO_SERVER_ARCHIVE(OnInstanceCommandCompleted)
			TO_SERVER_ARCHIVE(OnInstanceHeartbeatAcknowledged)
			TO_SERVER_ARCHIVE(OnInstanceLogOutput)
			TO_SERVER_ARCHIVE(OnInstanceTerminated)
			TO_SERVER_ARCHIVE(OnInstanceRecordingStopped)
			TO_SERVER_ARCHIVE(OnInstanceSaveStateCreated)
			TO_SERVER_ARCHIVE(OnInstanceMemoryRead)
			TO_SERVER_ARCHIVE(OnInstanceMemoryWrite)
			TO_SERVER_ARCHIVE(OnInstanceRenderGba)
		}
	}
};
