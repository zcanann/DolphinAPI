// Copyright 2018 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "dolphin-ipc/DolphinIpcHandlerBase.h"
#include "dolphin-ipc/IpcStructs.h"

#include "Common/Flag.h"
#include "Common/Logging/LogManager.h"
#include "Common/WindowSystemInfo.h"
#include "Core/Movie.h"

#include <memory>
#include <string>
#include <queue>

class InstanceIpcHandler;
class MockServer;

struct InstanceBootParameters
{
	std::string instanceId;
	bool recordOnLaunch = false;
	bool pauseOnBoot = true;
};

class Instance : public DolphinIpcHandlerBase, Common::Log::LogListener
{
public:
	Instance(const InstanceBootParameters& bootParams);
	virtual ~Instance();

	void InitializeLaunchOptions(const InstanceBootParameters& bootParams);
	bool IsRunning() const { return _running.IsSet(); }
	bool IsWindowFocused() const { return _window_focus; }
	bool IsWindowFullscreen() const { return _window_fullscreen; }
	virtual void SetTitle(const std::string& title);
	virtual void MainLoop() = 0;
	virtual u64 GetWindowIdentifier() const { return 0; }

	virtual bool Init();

	void InitControllers();
	void ShutdownControllers();
	void PrepareForTASInput();

	virtual WindowSystemInfo GetWindowSystemInfo() const = 0;

	// Requests a graceful shutdown, from SIGINT/SIGTERM.
	void RequestShutdown();

	// Request an immediate shutdown.
	void Stop();

	static std::unique_ptr<Instance> CreateHeadlessInstance(const InstanceBootParameters& bootParams);
#ifdef HAVE_X11
	static std::unique_ptr<Instance> CreateX11Instance(const InstanceBootParameters& bootParams);
#endif

#ifdef __linux__
	static std::unique_ptr<Instance> CreateFBDevInstance(const InstanceBootParameters& bootParams);
#endif

#ifdef _WIN32
	static std::unique_ptr<Instance> CreateWin32Instance(const InstanceBootParameters& bootParams);
#endif

protected:
	INSTANCE_FUNC_OVERRIDE(Connect);
	INSTANCE_FUNC_OVERRIDE(Heartbeat);
	INSTANCE_FUNC_OVERRIDE(Terminate);
	INSTANCE_FUNC_OVERRIDE(StartRecordingInput);
	INSTANCE_FUNC_OVERRIDE(StopRecordingInput);
	INSTANCE_FUNC_OVERRIDE(PauseEmulation);
	INSTANCE_FUNC_OVERRIDE(ResumeEmulation);
	INSTANCE_FUNC_OVERRIDE(PlayInputs);
	INSTANCE_FUNC_OVERRIDE(FrameAdvance);
	INSTANCE_FUNC_OVERRIDE(SetTasInput);
	INSTANCE_FUNC_OVERRIDE(CreateSaveState);
	INSTANCE_FUNC_OVERRIDE(LoadSaveState);
	INSTANCE_FUNC_OVERRIDE(LoadMemoryCardData);
	INSTANCE_FUNC_OVERRIDE(FormatMemoryCard);
	INSTANCE_FUNC_OVERRIDE(ReadMemory);
	INSTANCE_FUNC_OVERRIDE(WriteMemory);

	void CheckGcFrameAdvance(GCPadStatus* padStatus, int controllerId, bool checkInputs);
	void UpdateRunningFlag();
	void StartRecording();
	void StopRecording();
	void OnCommandCompleted(DolphinInstanceIpcCall completedCommand);
	void Log(Common::Log::LogLevel level, const char* text) override;

	Common::Flag _running{true};
	Common::Flag _shutdown_requested{false};
	Common::Flag _tried_graceful_shutdown{false};

	bool _window_focus = true;  // Should be made atomic if actually implemented
	bool _window_fullscreen = false;

	std::chrono::system_clock::time_point _lastHeartbeat = std::chrono::system_clock::now();

	enum class RecordingState
	{
		None,
		Recording,
		Playback,
	};

	int _coreStateEventHandle = -1;
	int _framesToAdvance = 0;
	bool _bootToPause = false;
	bool _shouldUseHardwareController = true;
	RecordingState _instanceState = RecordingState::None;

	bool _isRecordingController[4] = { true, false, false, false };
	DolphinInputRecording _recordingInputs[4];
	DolphinInputRecording _playbackInputs[4];
	DolphinControllerState _hardwareInputStates[4];
	DolphinControllerState _tasInputStates[4];

	std::shared_ptr<MockServer> _mockServer;
};
