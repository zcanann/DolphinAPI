#pragma once
// Defines the IPC methods that are called from the controlling library (server) to an individual Dolphin instance (client)

#include "external/cereal/cereal.hpp"

#include <string>

enum class DolphinInstanceIpcCall
{
    Null,
    DolphinInstance_Connect,
	DolphinInstance_StartRecordingInput,
	DolphinInstance_StopRecordingInput,
	DolphinInstance_PauseEmulation,
	DolphinInstance_UnpauseEmulation,
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

struct ToInstanceParams_UnpauseEmulation
{
	template <class Archive>
	void serialize(Archive& ar)
	{
	}
};

union DolphinIpcToInstanceDataParams
{
	DolphinIpcToInstanceDataParams() : _connectParams({}) { }
	~DolphinIpcToInstanceDataParams() {}

	std::shared_ptr<ToInstanceParams_Connect> _connectParams;
	std::shared_ptr<ToInstanceParams_StartRecordingInput> _startRecordingInputParams;
	std::shared_ptr<ToInstanceParams_StopRecordingInput> _stopRecordingInputParams;
	std::shared_ptr<ToInstanceParams_PauseEmulation> _pauseEmulationParams;
	std::shared_ptr<ToInstanceParams_UnpauseEmulation> _unpauseEmulationParams;
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
			case DolphinInstanceIpcCall::DolphinInstance_UnpauseEmulation:
			{
				if (!_params._unpauseEmulationParams)
				{
					_params._unpauseEmulationParams = std::make_shared<ToInstanceParams_UnpauseEmulation>();
				}
				ar(*(_params._unpauseEmulationParams));
				break;
			}
			case DolphinInstanceIpcCall::Null: default: break;
		}
	}
};
