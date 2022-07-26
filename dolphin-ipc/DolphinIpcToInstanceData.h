#pragma once
// Defines the IPC methods that are called from the controlling library (server) to an individual Dolphin instance (client)

#include "external/cereal/cereal.hpp"

#include <string>

enum class DolphinInstanceIpcCall
{
    Null,
    DolphinInstance_Connect,
	DolphinInstance_BeginRecordingInput,
	DolphinInstance_StopRecordingInput,
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

struct ToInstanceParams_BeginRecordingInput
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

union DolphinIpcToInstanceDataParams
{
	DolphinIpcToInstanceDataParams() : _connectParams({}) { }
	~DolphinIpcToInstanceDataParams() {}

	std::shared_ptr<ToInstanceParams_Connect> _connectParams;
	std::shared_ptr<ToInstanceParams_BeginRecordingInput> _beginRecordingInputParams;
	std::shared_ptr<ToInstanceParams_StopRecordingInput> _stopRecordingInputParams;
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
				if (!_params._connectParams)
				{
					_params._connectParams = std::make_shared<ToInstanceParams_Connect>();
				}
				ar(*(_params._connectParams));
				break;
			case DolphinInstanceIpcCall::DolphinInstance_BeginRecordingInput:
				if (!_params._beginRecordingInputParams)
				{
					_params._beginRecordingInputParams = std::make_shared<ToInstanceParams_BeginRecordingInput>();
				}
				ar(*(_params._beginRecordingInputParams));
				break;
			case DolphinInstanceIpcCall::DolphinInstance_StopRecordingInput:
				if (!_params._beginRecordingInputParams)
				{
					_params._stopRecordingInputParams = std::make_shared<ToInstanceParams_StopRecordingInput>();
				}
				ar(*(_params._stopRecordingInputParams));
				break;
			case DolphinInstanceIpcCall::Null: default: break;
		}
	}
};
