#pragma once
// Defines the IPC methods that are called from the controlling library (server) to an individual Dolphin instance (client)

#include "IpcStructs.h"

// Prevent errors in cereal that propagate to Unreal where __GNUC__ is not defined
#define __GNUC__ (false)
#include "external/cereal/cereal.hpp"
#undef __GNUC__

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
	DolphinInstance_SetTasInput,
	DolphinInstance_CreateSaveState,
	DolphinInstance_LoadSaveState,
	DolphinInstance_LoadMemoryCardData,
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
	bool _shouldUseHardwareController = true;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_shouldUseHardwareController);
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
	bool _unpauseInstance = true;
	bool _recordControllers[4] = { true, false, false, false };

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_unpauseInstance);
		ar(_recordControllers[0]);
		ar(_recordControllers[1]);
		ar(_recordControllers[2]);
		ar(_recordControllers[3]);
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

struct ToInstanceParams_SetTasInput
{
	DolphinControllerState _tasInputStates[4];

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_tasInputStates[0]);
		ar(_tasInputStates[1]);
		ar(_tasInputStates[2]);
		ar(_tasInputStates[3]);
	}
};

struct ToInstanceParams_CreateSaveState
{
	std::string _filePathNoExtension;
	bool _saveMemoryCards = true;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_filePathNoExtension);
		ar(_saveMemoryCards);
	}
};

struct ToInstanceParams_LoadSaveState
{
	std::string _saveFilePath;
	std::string _optionalMemoryCardDataAPath;
	std::string _optionalMemoryCardDataBPath;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_saveFilePath);
		ar(_optionalMemoryCardDataAPath);
		ar(_optionalMemoryCardDataBPath);
	}
};

struct ToInstanceParams_LoadMemoryCardData
{
	std::string _optionalMemoryCardDataAPath;
	std::string _optionalMemoryCardDataBPath;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(_optionalMemoryCardDataAPath);
		ar(_optionalMemoryCardDataBPath);
	}
};

struct ToInstanceParams_FormatMemoryCard
{
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
struct DolphinIpcToInstanceDataParams
{
	DolphinIpcToInstanceDataParams() {}
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
	TO_INSTANCE_MEMBER(SetTasInput)
	TO_INSTANCE_MEMBER(CreateSaveState)
	TO_INSTANCE_MEMBER(LoadSaveState)
	TO_INSTANCE_MEMBER(LoadMemoryCardData)
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
			TO_INSTANCE_ARCHIVE(SetTasInput)
			TO_INSTANCE_ARCHIVE(CreateSaveState)
			TO_INSTANCE_ARCHIVE(LoadSaveState)
			TO_INSTANCE_ARCHIVE(LoadMemoryCardData)
			TO_INSTANCE_ARCHIVE(FormatMemoryCard)
			TO_INSTANCE_ARCHIVE(ReadMemory)
			TO_INSTANCE_ARCHIVE(WriteMemory)
		}
	}
};
