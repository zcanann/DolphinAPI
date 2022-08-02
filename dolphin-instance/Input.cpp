#include "Input.h"

#include "InputCommon/GCPadStatus.h"

void Input::CopyControllerStateToGcPadStatus(const DolphinControllerState& padState, GCPadStatus* padStatus)
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

void Input::CopyGcPadStatusToControllerState(GCPadStatus* padStatus, DolphinControllerState& padState)
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
