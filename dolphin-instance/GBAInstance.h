// Copyright 2021 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <vector>

#include "Core/Host.h"

namespace HW::GBA
{
	class Core;
}

class Instance;
class GBAWidgetController;

class GBAInstance : public GBAHostInterface
{
public:
  explicit GBAInstance(std::weak_ptr<HW::GBA::Core> core, std::weak_ptr<Instance> instance);
  ~GBAInstance();

  void GameChanged() override;
  void FrameEnded(const std::vector<u32>& video_buffer) override;

private:
  GBAWidgetController* m_widget_controller{};
  std::weak_ptr<HW::GBA::Core> m_core;
  std::weak_ptr<Instance> m_instance;
};
