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
	DolphinInstance_FrameAdvanceWithInput,
	DolphinInstance_CreateSaveState,
	DolphinInstance_LoadSaveState,
	DolphinInstance_FormatMemoryCard,
	DolphinInstance_ImportGci,
	DolphinInstance_ReadMemory,
	DolphinInstance_WriteMemory,
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

struct ToInstanceParams_FrameAdvance
{
	int _numFrames = 1;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_numFrames);
	}
};

struct ToInstanceParams_FrameAdvanceWithInput
{
	int _numFrames = 1;
	DolphinControllerState _inputState[4];

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_numFrames);
		ar(_inputState[0]);
		ar(_inputState[1]);
		ar(_inputState[2]);
		ar(_inputState[3]);
	}
};

struct ToInstanceParams_CreateSaveState
{
	std::string _filePathNoExtension;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_filePathNoExtension);
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

struct ToInstanceParams_FormatMemoryCard
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
	DolphinSlot _slot = DolphinSlot::SlotA;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_cardSize);
		ar(_encoding);
		ar(_slot);
	}
};

struct ToInstanceParams_ReadMemory
{
	unsigned int _address;
	std::vector<int> _pointerOffsets;
	int _numberOfBytes = 0;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_address);
		ar(_pointerOffsets);
		ar(_numberOfBytes);
	}
};

struct ToInstanceParams_WriteMemory
{
	std::vector<unsigned char> _bytes;
	unsigned int _address;
	std::vector<int> _pointerOffsets;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_bytes);
		ar(_address);
		ar(_pointerOffsets);
	}
};

#define TO_INSTANCE_MEMBER(Name) std::shared_ptr<ToInstanceParams_##Name> _params ## Name;
union DolphinIpcToInstanceDataParams
{
	DolphinIpcToInstanceDataParams() : _paramsConnect({}) { }
	~DolphinIpcToInstanceDataParams() {}

	TO_INSTANCE_MEMBER(Connect)
	TO_INSTANCE_MEMBER(Heartbeat)
	TO_INSTANCE_MEMBER(Terminate)
	TO_INSTANCE_MEMBER(StartRecordingInput)
	TO_INSTANCE_MEMBER(StopRecordingInput)
	TO_INSTANCE_MEMBER(PauseEmulation)
	TO_INSTANCE_MEMBER(ResumeEmulation)
	TO_INSTANCE_MEMBER(PlayInputs)
	TO_INSTANCE_MEMBER(FrameAdvance)
	TO_INSTANCE_MEMBER(FrameAdvanceWithInput)
	TO_INSTANCE_MEMBER(CreateSaveState)
	TO_INSTANCE_MEMBER(LoadSaveState)
	TO_INSTANCE_MEMBER(FormatMemoryCard)
	TO_INSTANCE_MEMBER(ReadMemory)
	TO_INSTANCE_MEMBER(WriteMemory)
};

#define TO_INSTANCE_ARCHIVE(Name) case DolphinInstanceIpcCall::DolphinInstance_ ## Name: \
	{ \
		if (!_params._params ## Name) \
		_params._params ## Name = std::make_shared<ToInstanceParams_##Name>(); \
		ar(*(_params._params ## Name)); \
		break; \
	}

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
			TO_INSTANCE_ARCHIVE(Connect)
			TO_INSTANCE_ARCHIVE(Heartbeat)
			TO_INSTANCE_ARCHIVE(Terminate)
			TO_INSTANCE_ARCHIVE(StartRecordingInput)
			TO_INSTANCE_ARCHIVE(StopRecordingInput)
			TO_INSTANCE_ARCHIVE(PauseEmulation)
			TO_INSTANCE_ARCHIVE(ResumeEmulation)
			TO_INSTANCE_ARCHIVE(PlayInputs)
			TO_INSTANCE_ARCHIVE(FrameAdvance)
			TO_INSTANCE_ARCHIVE(FrameAdvanceWithInput)
			TO_INSTANCE_ARCHIVE(CreateSaveState)
			TO_INSTANCE_ARCHIVE(LoadSaveState)
			TO_INSTANCE_ARCHIVE(FormatMemoryCard)
			TO_INSTANCE_ARCHIVE(ReadMemory)
			TO_INSTANCE_ARCHIVE(WriteMemory)
		}
	}
};
