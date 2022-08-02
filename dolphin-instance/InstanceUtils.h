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
};
