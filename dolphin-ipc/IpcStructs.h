#pragma once

#include "external/cereal/cereal.hpp"

enum class DolphinSlot
{
    SlotA,
    SlotB,
    SP1,
};

struct DolphinControllerState
{
    bool Start, A, B, X, Y, Z;  // Binary buttons, 6 bits
    bool DPadUp, DPadDown, DPadLeft, DPadRight; // Binary D-Pad buttons, 4 bits
    bool L, R;      // Binary triggers, 2 bits
    bool Disc;          // Checks for disc being changed
    bool Reset;         // Console reset button
    bool IsConnected;  // Should controller be treated as connected
    bool GetOrigin;    // Special bit to indicate analog origin reset
    unsigned char TriggerL, TriggerR;          // Triggers, 8 bits
    unsigned char AnalogStickX, AnalogStickY;  // Main Stick, 8 bits
    unsigned char CStickX, CStickY;            // Sub-Stick, 8 bits

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(Start);
        ar(A);
        ar(B);
        ar(X);
        ar(Y);
        ar(Z);
        ar(DPadUp);
        ar(DPadDown);
        ar(DPadLeft);
        ar(DPadRight);
        ar(L);
        ar(R);
        ar(Disc);
        ar(Reset);
        ar(IsConnected);
        ar(GetOrigin);
        ar(TriggerL);
        ar(TriggerR);
        ar(AnalogStickX);
        ar(AnalogStickY);
        ar(CStickX);
        ar(CStickY);
    }
};
