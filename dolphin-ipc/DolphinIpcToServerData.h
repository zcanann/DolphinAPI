#pragma once
// Defines the IPC callbacks that are called from an individual Dolphin instance (client) to the controlling library (server)

#include "IpcStructs.h"

#include "external/cereal/cereal.hpp"

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
	DolphinServer_OnInstanceMemoryCardFormatted,
};

struct ToServerParams_OnInstanceConnected
{
	template <class Archive>
	void serialize(Archive& ar)
	{
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

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_isRecording);
		ar(_isPaused);
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
	std::vector<DolphinControllerState> _inputStates;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_inputStates);
	}
};

struct ToServerParams_OnInstanceSaveStateCreated
{
	std::string _filePathNoExtension;
	std::vector<DolphinControllerState> _recordingInputs;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_filePathNoExtension);
		ar(_recordingInputs);
	}
};

struct ToServerParams_OnInstanceMemoryCardFormatted
{
	DolphinSlot _slot;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_slot);
	}
};

#define TO_SERVER_MEMBER(Name) std::shared_ptr<ToServerParams_##Name> _params ## Name;
union DolphinIpcToServerDataParams
{
	DolphinIpcToServerDataParams() : _paramsOnInstanceConnected({}) { }
	~DolphinIpcToServerDataParams() {}

	TO_SERVER_MEMBER(OnInstanceConnected)
	TO_SERVER_MEMBER(OnInstanceCommandCompleted)
	TO_SERVER_MEMBER(OnInstanceHeartbeatAcknowledged)
	TO_SERVER_MEMBER(OnInstanceLogOutput)
	TO_SERVER_MEMBER(OnInstanceTerminated)
	TO_SERVER_MEMBER(OnInstanceRecordingStopped)
	TO_SERVER_MEMBER(OnInstanceSaveStateCreated)
	TO_SERVER_MEMBER(OnInstanceMemoryCardFormatted)
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
			TO_SERVER_ARCHIVE(OnInstanceMemoryCardFormatted)
		}
	}
};
