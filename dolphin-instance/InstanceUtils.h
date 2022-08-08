#pragma once

#include "dolphin-ipc/IpcStructs.h"

struct GCPadStatus;

class InstanceUtils
{
public:
	static void CopyControllerStateToGcPadStatus(const DolphinControllerState& padState, GCPadStatus* padStatus);
	static void CopyGcPadStatusToControllerState(GCPadStatus* padStatus, DolphinControllerState& padState);

	static std::string GetPathForMemoryCardSlot(DolphinSlot slot);
	static bool ExportGci(DolphinSlot slot, const std::string& filePath);

	static u32 ResolvePointer(u32 address, std::vector<s32> offsets);
	static std::vector<u8> ReadBytes(u32 address, s32 numberOfBytes);
	static void WriteBytes(u32 address, std::vector<u8> bytes);

private:
	static u8* GetPointerForRange(u32 address, size_t size);
	static u8* GetPointer(u32 address);
};
