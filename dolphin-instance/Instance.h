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

class InstanceIpcHandler;

class Instance : public DolphinIpcHandlerBase
{
public:
	Instance(const std::string& instanceId);
	virtual ~Instance();

	bool IsRunning() const { return _running.IsSet(); }
	bool IsWindowFocused() const { return _window_focus; }
	bool IsWindowFullscreen() const { return _window_fullscreen; }
	virtual void SetTitle(const std::string& title);
	virtual void MainLoop() = 0;

	virtual bool Init();

	void InitControllers();
	void ShutdownControllers();
	void PrepareForTASInput();

	virtual WindowSystemInfo GetWindowSystemInfo() const = 0;

	// Requests a graceful shutdown, from SIGINT/SIGTERM.
	void RequestShutdown();

	// Request an immediate shutdown.
	void Stop();

	static std::unique_ptr<Instance> CreateHeadlessInstance(const std::string& instanceId);
#ifdef HAVE_X11
	static std::unique_ptr<Instance> CreateX11Instance(const std::string& instanceId);
#endif

#ifdef __linux__
	static std::unique_ptr<Instance> CreateFBDevInstance(const std::string& instanceId);
#endif

#ifdef _WIN32
	static std::unique_ptr<Instance> CreateWin32Instance(const std::string& instanceId);
#endif

protected:
	virtual void DolphinInstance_Connect(const ToInstanceParams_Connect& connectParams) override;
	virtual void DolphinInstance_BeginRecordingInput(const ToInstanceParams_BeginRecordingInput& beginRecordingInputParams) override;
	virtual void DolphinInstance_StopRecordingInput(const ToInstanceParams_StopRecordingInput& stopRecordingInputParams) override;

	void UpdateRunningFlag();

	Common::Flag _running{true};
	Common::Flag _shutdown_requested{false};
	Common::Flag _tried_graceful_shutdown{false};

	bool _window_focus = true;  // Should be made atomic if actually implemented
	bool _window_fullscreen = false;

	bool _isRecording = false;
	std::vector<DolphinControllerState> _inputs;
};
