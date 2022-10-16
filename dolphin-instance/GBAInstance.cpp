// Copyright 2021 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "GBAInstance.h"

#include "Core/HW/GBACore.h"

GBAInstance::GBAInstance(std::weak_ptr<HW::GBA::Core> core)
{
  m_core = std::move(core);
}

GBAInstance::~GBAInstance()
{
}

void GBAInstance::GameChanged()
{
  auto core_ptr = m_core.lock();
  if (!core_ptr || !core_ptr->IsStarted())
  {
      return;
  }
}

void GBAInstance::FrameEnded(const std::vector<u32>& video_buffer)
{
    // Send the GBA video over IPC. This saves the effort of needing to create an entire window.
    std::shared_ptr<HW::GBA::Core> core_ptr = m_core.lock();
    HW::GBA::CoreInfo m_core_info = core_ptr->GetCoreInfo();

    if (video_buffer.size() == static_cast<size_t>(m_core_info.width * m_core_info.height))
    {

    }
}

std::unique_ptr<GBAHostInterface> Host_CreateGBAInstance(std::weak_ptr<HW::GBA::Core> core)
{
  return std::make_unique<GBAInstance>(core);
}
