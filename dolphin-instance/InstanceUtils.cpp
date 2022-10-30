#include "InstanceUtils.h"

#include "InputCommon/GCPadStatus.h"

#include "Common/FileUtil.h"
#include "Common/VariantUtil.h"
#include "Core/Config/MainSettings.h"
#include "Core/Config/WiimoteSettings.h"
#include "Core/ConfigManager.h"
#include "Core/HW/GCMemcard/GCMemcard.h"
#include "Core/HW/GCMemcard/GCMemcardUtils.h"
#include "Core/HW/Memmap.h"

#include <variant>

void InstanceUtils::CopyControllerStateToGcPadStatus(const DolphinControllerState& padState, GCPadStatus* padStatus)
{
    if (padStatus == nullptr)
    {
        return;
    }

    padStatus->isConnected = padState.IsConnected;

    padStatus->triggerLeft = padState.TriggerL;
    padStatus->triggerRight = padState.TriggerR;

    padStatus->stickX = padState.AnalogStickX;
    padStatus->stickY = padState.AnalogStickY;

    padStatus->substickX = padState.CStickX;
    padStatus->substickY = padState.CStickY;

    padStatus->button = 0;
    padStatus->button |= PAD_USE_ORIGIN;
    padStatus->button |= padState.A ? PAD_BUTTON_A : 0;
    padStatus->analogA = padState.A ? 0xFF : 0x00;
    padStatus->button |= padState.B ? PAD_BUTTON_B : 0;
    padStatus->analogB = padState.B ? 0xFF : 0x00;
    padStatus->button |= padState.X ? PAD_BUTTON_X : 0;
    padStatus->button |= padState.Y ? PAD_BUTTON_Y : 0;
    padStatus->button |= padState.Z ? PAD_TRIGGER_Z : 0;
    padStatus->button |= padState.Start ? PAD_BUTTON_START : 0;
    padStatus->button |= padState.DPadUp ? PAD_BUTTON_UP : 0;
    padStatus->button |= padState.DPadDown ? PAD_BUTTON_DOWN : 0;
    padStatus->button |= padState.DPadLeft ? PAD_BUTTON_LEFT : 0;
    padStatus->button |= padState.DPadRight ? PAD_BUTTON_RIGHT : 0;
    padStatus->button |= padState.L ? PAD_TRIGGER_L : 0;
    padStatus->button |= padState.R ? PAD_TRIGGER_R : 0;
    padStatus->button |= padState.GetOrigin ? PAD_GET_ORIGIN : 0;
}

void InstanceUtils::CopyGcPadStatusToControllerState(GCPadStatus* padStatus, DolphinControllerState& padState)
{
    padState.A = ((padStatus->button & PAD_BUTTON_A) != 0);
    padState.B = ((padStatus->button & PAD_BUTTON_B) != 0);
    padState.X = ((padStatus->button & PAD_BUTTON_X) != 0);
    padState.Y = ((padStatus->button & PAD_BUTTON_Y) != 0);
    padState.Z = ((padStatus->button & PAD_TRIGGER_Z) != 0);
    padState.Start = ((padStatus->button & PAD_BUTTON_START) != 0);

    padState.DPadUp = ((padStatus->button & PAD_BUTTON_UP) != 0);
    padState.DPadDown = ((padStatus->button & PAD_BUTTON_DOWN) != 0);
    padState.DPadLeft = ((padStatus->button & PAD_BUTTON_LEFT) != 0);
    padState.DPadRight = ((padStatus->button & PAD_BUTTON_RIGHT) != 0);

    padState.L = ((padStatus->button & PAD_TRIGGER_L) != 0);
    padState.R = ((padStatus->button & PAD_TRIGGER_R) != 0);
    padState.TriggerL = padStatus->triggerLeft;
    padState.TriggerR = padStatus->triggerRight;

    padState.AnalogStickX = padStatus->stickX;
    padState.AnalogStickY = padStatus->stickY;

    padState.CStickX = padStatus->substickX;
    padState.CStickY = padStatus->substickY;

    padState.IsConnected = padStatus->isConnected;

    padState.GetOrigin = (padStatus->button & PAD_GET_ORIGIN) != 0;

    padState.Disc = false; // TODO
    padState.Reset = false; // TODO
}

std::string InstanceUtils::GetPathForMemoryCardSlot(DolphinSlot slot)
{
    switch (slot)
    {
        case DolphinSlot::SlotA:
        {
            return Config::Get(Config::MAIN_MEMCARD_A_PATH);
        }
        case DolphinSlot::SlotB:
        {
            return Config::Get(Config::MAIN_MEMCARD_A_PATH);
        }
        default:
        {
            return "";
        }
    }
}

bool InstanceUtils::ExportGci(DolphinSlot slot, const std::string& filePath)
{
    if (filePath.empty())
    {
        return false;
    }

    if (File::Exists(filePath))
    {
        File::Delete(filePath);
    }

    std::string slotPath = InstanceUtils::GetPathForMemoryCardSlot(DolphinSlot::SlotA);

    // Read the current gamecode from memory
    std::string gameCode =
    {
        (char)Memory::Read_U8(0x80000000),
        (char)Memory::Read_U8(0x80000001),
        (char)Memory::Read_U8(0x80000002),
        (char)Memory::Read_U8(0x80000003),
        // (char)Memory::Read_U8(0x80000004),
        // (char)Memory::Read_U8(0x80000005),
    };

    std::pair<Memcard::GCMemcardErrorCode, std::optional<Memcard::GCMemcard>> memoryCardOpenInfo = Memcard::GCMemcard::Open(slotPath);

    if (memoryCardOpenInfo.second.has_value())
    {
        Memcard::GCMemcard& memoryCard = memoryCardOpenInfo.second.value();
        const u8 numFiles = memoryCard.GetNumFiles();
        for (int index = 0; index < numFiles; index++)
        {
            const u8 fileIndex = memoryCard.GetFileIndex(index);
            std::string memoryCardGameCode = memoryCard.DEntry_GameCode(fileIndex);

            if (memoryCardGameCode != gameCode)
            {
                break;
            }

            const std::vector<Memcard::Savefile> savefiles = Memcard::GetSavefiles(memoryCard, { fileIndex });

            if (savefiles.size() <= 1)
            {
                if (!Memcard::WriteSavefile(filePath, savefiles[0], Memcard::SavefileFormat::GCI))
                {
                    File::Delete(filePath);
                }
            }
        }
    }

    return true;
}

bool InstanceUtils::ImportGci(DolphinSlot slot, const std::string& filePath)
{
    if (filePath.empty())
    {
        return false;
    }

    std::string slotPath = InstanceUtils::GetPathForMemoryCardSlot(DolphinSlot::SlotA);
    std::pair<Memcard::GCMemcardErrorCode, std::optional<Memcard::GCMemcard>> memoryCardOpenInfo = Memcard::GCMemcard::Open(slotPath);
    Memcard::GCMemcard& memoryCard = memoryCardOpenInfo.second.value();
    std::variant<Memcard::ReadSavefileErrorCode, Memcard::Savefile> readResult = Memcard::ReadSavefile(filePath);

    bool result = false;

    std::visit(overloaded
    {
        [&](Memcard::Savefile gci)
        {
            Memcard::GCMemcardImportFileRetVal importResult = memoryCard.ImportFile(gci);
            result = importResult == Memcard::GCMemcardImportFileRetVal::SUCCESS;
        },
        [&](Memcard::ReadSavefileErrorCode error_code)
        {
            result = false;
        },
    },
    std::move(readResult));

    return true;
}

u32 InstanceUtils::ResolvePointer(u32 address, std::vector<s32> offsets)
{
    for (unsigned long long offset : offsets)
    {
        if (void* pointer = InstanceUtils::GetPointerForRange(address, sizeof(address)))
        {
            memcpy(&address, pointer, sizeof(address));
            address = Common::swap32(address);
            address += offset;
        }
    }

    return address;
}

std::vector<u8> InstanceUtils::ReadBytes(u32 address, s32 numberOfBytes)
{
    std::vector<u8> bytes = std::vector<u8>(numberOfBytes);
    void* pointer = InstanceUtils::GetPointerForRange(address, numberOfBytes);

    if (pointer)
    {
        memcpy(bytes.data(), pointer, numberOfBytes);
    }

    return bytes;
}

void InstanceUtils::WriteBytes(u32 address, std::vector<u8> bytes)
{
    void* pointer = InstanceUtils::GetPointerForRange(address, bytes.size());

    if (pointer)
    {
        memcpy(pointer, bytes.data(), bytes.size());
    }
}

u8* InstanceUtils::GetPointerForRange(u32 address, size_t size)
{
    // Make sure we don't have a range spanning 2 separate banks
    if (size <= 0 || size >= Memory::GetExRamSizeReal())
    {;
        return nullptr;
    }

    // Check that the beginning and end of the range are valid
    u8* pointer = InstanceUtils::GetPointer(address);
    if (!pointer || !InstanceUtils::GetPointer(address + u32(size) - 1))
    {
        // A panic alert has already been raised by GetPointer
        return nullptr;
    }

    return pointer;
}

// Reimplementation of Memory::GetPointer() with no panics
u8* InstanceUtils::GetPointer(u32 address)
{
    address &= 0x3FFFFFFF;
    if (address < Memory::GetRamSizeReal())
    {
        return Memory::m_pRAM + address;
    }

    if (Memory::m_pEXRAM)
    {
        if ((address >> 28) == 0x1 && (address & 0x0fffffff) < Memory::GetExRamSizeReal())
        {
            return Memory::m_pEXRAM + (address & Memory::GetExRamMask());
        }
    }

    return nullptr;
}
