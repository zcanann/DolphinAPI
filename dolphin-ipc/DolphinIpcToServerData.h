#pragma once
// Defines the IPC callbacks that are called from an individual Dolphin instance (client) to the controlling library (server)

#include "IpcStructs.h"

#include "external/cereal/cereal.hpp"

#include <string>

enum class DolphinServerIpcCall
{
	Null,
	DolphinServer_OnInstanceConnected,
	DolphinServer_OnInstanceReady,
	DolphinServer_OnInstanceHeartbeatAcknowledged,
	DolphinServer_OnInstanceTerminated,
	DolphinServer_OnInstanceRecordingStopped,
};

struct ToServerParams_OnInstanceConnected
{
	template <class Archive>
	void serialize(Archive& ar)
	{
	}
};

struct ToServerParams_OnInstanceReady
{
	template <class Archive>
	void serialize(Archive& ar)
	{
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

struct ToServerParams_OnInstanceTerminated
{
	std::string _params1;
	std::string _params2;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_params1);
		ar(_params2);
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

union DolphinIpcToServerDataParams
{
	DolphinIpcToServerDataParams() : _onInstanceConnectedParams({}) { }
	~DolphinIpcToServerDataParams() {}

	std::shared_ptr<ToServerParams_OnInstanceConnected> _onInstanceConnectedParams;
	std::shared_ptr<ToServerParams_OnInstanceReady> _onInstanceReadyParams;
	std::shared_ptr<ToServerParams_OnInstanceHeartbeatAcknowledged> _onInstanceHeartbeatAcknowledged;
	std::shared_ptr<ToServerParams_OnInstanceTerminated> _onInstanceTerminatedParams;
	std::shared_ptr<ToServerParams_OnInstanceRecordingStopped> _onInstanceRecordingStopped;
};

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
			case DolphinServerIpcCall::DolphinServer_OnInstanceConnected:
			{
				if (!_params._onInstanceConnectedParams)
				{
					_params._onInstanceConnectedParams = std::make_shared<ToServerParams_OnInstanceConnected>();
				}
				ar(*(_params._onInstanceConnectedParams));
				break;
			}
			case DolphinServerIpcCall::DolphinServer_OnInstanceReady:
			{
				if (!_params._onInstanceReadyParams)
				{
					_params._onInstanceReadyParams = std::make_shared<ToServerParams_OnInstanceReady>();
				}
				ar(*(_params._onInstanceReadyParams));
				break;
			}
			case DolphinServerIpcCall::DolphinServer_OnInstanceHeartbeatAcknowledged:
			{
				if (!_params._onInstanceHeartbeatAcknowledged)
				{
					_params._onInstanceHeartbeatAcknowledged = std::make_shared<ToServerParams_OnInstanceHeartbeatAcknowledged>();
				}
				ar(*(_params._onInstanceHeartbeatAcknowledged));
				break;
			}
			case DolphinServerIpcCall::DolphinServer_OnInstanceTerminated:
			{
				if (!_params._onInstanceTerminatedParams)
				{
					_params._onInstanceTerminatedParams = std::make_shared<ToServerParams_OnInstanceTerminated>();
				}
				ar(*(_params._onInstanceTerminatedParams));
				break;
			}
			case DolphinServerIpcCall::DolphinServer_OnInstanceRecordingStopped:
			{
				if (!_params._onInstanceRecordingStopped)
				{
					_params._onInstanceRecordingStopped = std::make_shared<ToServerParams_OnInstanceRecordingStopped>();
				}
				ar(*(_params._onInstanceRecordingStopped));
				break;
			}
			case DolphinServerIpcCall::Null: default: break;
		}
	}
};
