// Copyright 2018 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "dolphin-ipc/DolphinIpcHandlerBase.h"
#include "dolphin-ipc/IpcStructs.h"

#include "Common/Flag.h"
#include "Common/WindowSystemInfo.h"
#include "Core/Movie.h"

#include <memory>
#include <string>
#include <queue>

class InstanceIpcHandler;

class MockServer : public DolphinIpcHandlerBase
{
public:
	MockServer(const std::string& instanceId);
	virtual ~MockServer();

	void Update();

protected:
	virtual void DolphinServer_OnInstanceConnected(const ToServerParams_OnInstanceConnected& onInstanceConnectedParams) override;
	virtual void DolphinServer_OnInstanceHeartbeatAcknowledged(const ToServerParams_OnInstanceHeartbeatAcknowledged& onInstanceHeartbeatAcknowledgedParams) override;
	virtual void DolphinServer_OnInstanceTerminated(const ToServerParams_OnInstanceTerminated& onInstanceTerminatedParams) override;
	virtual void DolphinServer_OnInstanceRecordingStopped(const ToServerParams_OnInstanceRecordingStopped& onInstanceRecordingStopped) override;
};
