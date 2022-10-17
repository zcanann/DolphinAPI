// Copyright 2021 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "GBAInstance.h"

#include "dolphin-ipc/DolphinIpcHandlerBase.h"
#include "dolphin-ipc/external/jpeg-compressor/jpge.h"
#include "dolphin-ipc/IpcStructs.h"

#include "Instance.h"

#include "Core/HW/GBACore.h"
#include "Common/Random.h"

GBAInstance::GBAInstance(std::weak_ptr<HW::GBA::Core> core, std::weak_ptr<Instance> instance)
{
    m_core = std::move(core);
    m_instance = std::move(instance);
}

GBAInstance::~GBAInstance()
{
}

void GBAInstance::GameChanged()
{
  std::shared_ptr<HW::GBA::Core> core_ptr = m_core.lock();
  if (!core_ptr || !core_ptr->IsStarted())
  {
      return;
  }
}

void GBAInstance::FrameEnded(const std::vector<u32>& video_buffer)
{
    std::shared_ptr<HW::GBA::Core> core_ptr = m_core.lock();
    std::shared_ptr<Instance> instanc_ptr = m_instance.lock();

    if (--throttle > 0)
    {
        return;
    }

    if (core_ptr && instanc_ptr)
    {
        HW::GBA::CoreInfo m_core_info = core_ptr->GetCoreInfo();

        // Send the GBA frame render over IPC. This saves the effort of needing to create an entire window, since the server can render to texture.
        // This also has an added benefit of allowing the TAS to rapidly switch controller devices without a major performance hit.
        CREATE_TO_SERVER_DATA(OnInstanceRenderGba, ipcData, data);
        data->_controllerIndex = m_core_info.device_number;
        data->_width = m_core_info.width;
        data->_height = m_core_info.height;

        // (Optionally) Compress video buffer
        static const bool UseJpgCompression = false;

        if (UseJpgCompression)
        {
            // Set to the same size as the video buffer, but we shouldn't end up using it all due to jpg compression
            data->_frameBuffer.resize(video_buffer.size());

            int bufferSize = int(data->_frameBuffer.size());
            jpge::compress_image_to_jpeg_file_in_memory(data->_frameBuffer.data(), bufferSize, m_core_info.width, m_core_info.height, 4,
                reinterpret_cast<const jpge::uint8*>(video_buffer.data()), jpge::params());
            data->_frameBuffer.resize(bufferSize);
            data->_compressed = true;
        }
        else
        {
            data->_frameBuffer = video_buffer;
            data->_compressed = false;
        }
        
        instanc_ptr->ipcSendToServer(ipcData);
    }

    throttle = 30 + Common::Random::GenerateValue<u8>() % 3;
}
