#pragma once
// Defines the IPC methods that are called from the controlling library (server) to an individual Dolphin instance (client)

#include "IpcStructs.h"

#include "external/cereal/cereal.hpp"

#include <string>

enum class DolphinInstanceIpcCall
{
    Null,
    DolphinInstance_Connect,
	DolphinInstance_Heartbeat,
	DolphinInstance_Terminate,
	DolphinInstance_StartRecordingInput,
	DolphinInstance_StopRecordingInput,
	DolphinInstance_PauseEmulation, 
	DolphinInstance_ResumeEmulation,
	DolphinInstance_PlayInputs,
};

struct ToInstanceParams_Connect
{
    std::string _channelName;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_channelName);
	}
};

struct ToInstanceParams_Heartbeat
{
	template <class Archive>
	void serialize(Archive& ar)
	{
	}
};

struct ToInstanceParams_Terminate
{
	template <class Archive>
	void serialize(Archive& ar)
	{
	}
};

struct ToInstanceParams_StartRecordingInput
{
	template <class Archive>
	void serialize(Archive& ar)
	{
	}
};

struct ToInstanceParams_StopRecordingInput
{
	template <class Archive>
	void serialize(Archive& ar)
	{
	}
};

struct ToInstanceParams_PauseEmulation
{
	template <class Archive>
	void serialize(Archive& ar)
	{
	}
};

struct ToInstanceParams_ResumeEmulation
{
	template <class Archive>
	void serialize(Archive& ar)
	{
	}
};

struct ToInstanceParams_PlayInputs
{
	std::vector<DolphinControllerState> _inputStates;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_inputStates);
	}
};

union DolphinIpcToInstanceDataParams
{
	DolphinIpcToInstanceDataParams() : _connectParams({}) { }
	~DolphinIpcToInstanceDataParams() {}

	std::shared_ptr<ToInstanceParams_Connect> _connectParams;
	std::shared_ptr<ToInstanceParams_Heartbeat> _heartbeatParams;
	std::shared_ptr<ToInstanceParams_Terminate> _terminateParams;
	std::shared_ptr<ToInstanceParams_StartRecordingInput> _startRecordingInputParams;
	std::shared_ptr<ToInstanceParams_StopRecordingInput> _stopRecordingInputParams;
	std::shared_ptr<ToInstanceParams_PauseEmulation> _pauseEmulationParams;
	std::shared_ptr<ToInstanceParams_ResumeEmulation> _resumeEmulationParams;
	std::shared_ptr<ToInstanceParams_PlayInputs> _playInputsParams;
};

struct DolphinIpcToInstanceData
{
    DolphinInstanceIpcCall _call = DolphinInstanceIpcCall::Null;
	DolphinIpcToInstanceDataParams _params;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_call);

		switch (_call)
		{
			case DolphinInstanceIpcCall::DolphinInstance_Connect:
			{
				if (!_params._connectParams)
				{
					_params._connectParams = std::make_shared<ToInstanceParams_Connect>();
				}
				ar(*(_params._connectParams));
				break;
			}
			case DolphinInstanceIpcCall::DolphinInstance_Heartbeat:
			{
				if (!_params._heartbeatParams)
				{
					_params._heartbeatParams = std::make_shared<ToInstanceParams_Heartbeat>();
				}
				ar(*(_params._heartbeatParams));
				break;
			}
			case DolphinInstanceIpcCall::DolphinInstance_Terminate:
			{
				if (!_params._terminateParams)
				{
					_params._terminateParams = std::make_shared<ToInstanceParams_Terminate>();
				}
				ar(*(_params._terminateParams));
				break;
			}
			case DolphinInstanceIpcCall::DolphinInstance_StartRecordingInput:
			{
				if (!_params._startRecordingInputParams)
				{
					_params._startRecordingInputParams = std::make_shared<ToInstanceParams_StartRecordingInput>();
				}
				ar(*(_params._startRecordingInputParams));
				break;
			}
			case DolphinInstanceIpcCall::DolphinInstance_StopRecordingInput:
			{
				if (!_params._stopRecordingInputParams)
				{
					_params._stopRecordingInputParams = std::make_shared<ToInstanceParams_StopRecordingInput>();
				}
				ar(*(_params._stopRecordingInputParams));
				break;
			}
			case DolphinInstanceIpcCall::DolphinInstance_PauseEmulation:
			{
				if (!_params._pauseEmulationParams)
				{
					_params._pauseEmulationParams = std::make_shared<ToInstanceParams_PauseEmulation>();
				}
				ar(*(_params._pauseEmulationParams));
				break;
			}
			case DolphinInstanceIpcCall::DolphinInstance_ResumeEmulation:
			{
				if (!_params._resumeEmulationParams)
				{
					_params._resumeEmulationParams = std::make_shared<ToInstanceParams_ResumeEmulation>();
				}
				ar(*(_params._resumeEmulationParams));
				break;
			}
			case DolphinInstanceIpcCall::DolphinInstance_PlayInputs:
			{
				if (!_params._playInputsParams)
				{
					_params._playInputsParams = std::make_shared<ToInstanceParams_PlayInputs>();
				}
				ar(*(_params._playInputsParams));
				break;
			}
			case DolphinInstanceIpcCall::Null: default: break;
		}
	}
};
