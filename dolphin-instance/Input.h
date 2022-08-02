#pragma once

#include "dolphin-ipc/IpcStructs.h"

struct GCPadStatus;

class Input
{
public:
	static void CopyControllerStateToGcPadStatus(const DolphinControllerState& padState, GCPadStatus* padStatus);
	static void CopyGcPadStatusToControllerState(GCPadStatus* padStatus, DolphinControllerState& padState);
};
