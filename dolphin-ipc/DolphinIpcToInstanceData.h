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
	DolphinInstance_FrameAdvance,
	DolphinInstance_CreateSaveState,
	DolphinInstance_LoadSaveState,
	DolphinInstance_CreateMemoryCard,
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

struct ToInstanceParams_FrameAdvance
{
	int _numFrames = 1;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_numFrames);
	}
};

struct ToInstanceParams_CreateSaveState
{
	std::string _filePath;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_filePath);
	}
};

struct ToInstanceParams_LoadSaveState
{
	std::string _filePath;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_filePath);
	}
};

/*

  m_combobox_size->addItem(tr("4 Mbit (59 blocks)"), 4);
  m_combobox_size->addItem(tr("8 Mbit (123 blocks)"), 8);
  m_combobox_size->addItem(tr("16 Mbit (251 blocks)"), 16);
  m_combobox_size->addItem(tr("32 Mbit (507 blocks)"), 32);
  m_combobox_size->addItem(tr("64 Mbit (1019 blocks)"), 64);
  m_combobox_size->addItem(tr("128 Mbit (2043 blocks)"), 128);
*/

struct ToInstanceParams_CreateMemoryCard
{
	enum class CardSize
	{
		GC_4_Mbit_59_Blocks,
		GC_8_Mbit_123_Blocks,
		GC_16_Mbit_251_Blocks,
		GC_32_Mbit_507_Blocks,
		GC_64_Mbit_1019_Blocks,
		GC_128_Mbit_2043_Blocks,
	};
	enum class CardEncoding
	{
		Western,
		Japanese,
	};

	CardSize _cardSize = CardSize::GC_128_Mbit_2043_Blocks;
	CardEncoding _encoding = CardEncoding::Western;
	std::string _filePath;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_filePath);
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
	std::shared_ptr<ToInstanceParams_FrameAdvance> _frameAdvanceParams;
	std::shared_ptr<ToInstanceParams_CreateSaveState> _createSaveStateParams;
	std::shared_ptr<ToInstanceParams_LoadSaveState> _loadSaveStateParams;
	std::shared_ptr<ToInstanceParams_CreateMemoryCard> _createMemoryCardParams;
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
			case DolphinInstanceIpcCall::DolphinInstance_FrameAdvance:
			{
				if (!_params._frameAdvanceParams)
				{
					_params._frameAdvanceParams = std::make_shared<ToInstanceParams_FrameAdvance>();
				}
				ar(*(_params._frameAdvanceParams));
				break;
			}
			case DolphinInstanceIpcCall::DolphinInstance_CreateSaveState:
			{
				if (!_params._createSaveStateParams)
				{
					_params._createSaveStateParams = std::make_shared<ToInstanceParams_CreateSaveState>();
				}
				ar(*(_params._createSaveStateParams));
				break;
			}
			case DolphinInstanceIpcCall::DolphinInstance_LoadSaveState:
			{
				if (!_params._loadSaveStateParams)
				{
					_params._loadSaveStateParams = std::make_shared<ToInstanceParams_LoadSaveState>();
				}
				ar(*(_params._loadSaveStateParams));
				break;
			}
			case DolphinInstanceIpcCall::DolphinInstance_CreateMemoryCard:
			{
				if (!_params._createMemoryCardParams)
				{
					_params._createMemoryCardParams = std::make_shared<ToInstanceParams_CreateMemoryCard>();
				}
				ar(*(_params._createMemoryCardParams));
				break;
			}
			case DolphinInstanceIpcCall::Null: default: break;
		}
	}
};
