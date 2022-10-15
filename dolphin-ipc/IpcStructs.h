#pragma once

// Prevent errors in cereal that propagate to Unreal where __GNUC__ is not defined
#define __GNUC__ (false)
#include "external/cereal/cereal.hpp"
#include "external/cereal/types/vector.hpp"
#undef __GNUC__

#include <functional>
#include <numeric>

template <typename T, typename U>
bool AllEqual(const T& t, const U& u)
{
    return t == u;
}

template <typename T, typename U, typename... Others>
bool AllEqual(const T& t, const U& u, Others const &... args)
{
    return (t == u) && AllEqual(u, args...);
}

enum class DolphinSlot
{
    SlotA,
    SlotB,
    SP1,
};

struct DolphinControllerState
{
    bool Start, A, B, X, Y, Z = false;  // Binary buttons, 6 bits
    bool DPadUp, DPadDown, DPadLeft, DPadRight = false; // Binary D-Pad buttons, 4 bits
    bool L, R = false;          // Binary triggers, 2 bits
    bool Disc = false;          // Checks for disc being changed
    bool Reset = false;         // Console reset button
    bool IsConnected = false;   // Should controller be treated as connected
    bool GetOrigin = false;     // Special bit to indicate analog origin reset
    unsigned char TriggerL, TriggerR = 0;          // Triggers, 8 bits
    unsigned char AnalogStickX, AnalogStickY = 0;  // Main Stick, 8 bits
    unsigned char CStickX, CStickY = 0;            // Sub-Stick, 8 bits

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

struct ButtonRunLengthEncoded
{
    bool Pressed = false;
    int Length = 0;

    ButtonRunLengthEncoded(bool InPressed = false, int InLength = 0) : Pressed(InPressed), Length(InLength) { }

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(Pressed);
        ar(Length);
    }
};

struct AnalogRunLengthEncoded
{
    unsigned char Value = 0;
    int Length = 0;

    AnalogRunLengthEncoded(unsigned char InValue = 0, int InLength = 0) : Value(InValue), Length(InLength) { }

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(Value);
        ar(Length);
    }
};

struct DolphinInputRecording
{
    std::vector<ButtonRunLengthEncoded> Start, A, B, X, Y, Z;
    std::vector<ButtonRunLengthEncoded> DPadUp, DPadDown, DPadLeft, DPadRight;
    std::vector<ButtonRunLengthEncoded> L, R;
    std::vector<ButtonRunLengthEncoded> Disc;
    std::vector<ButtonRunLengthEncoded> Reset;
    std::vector<ButtonRunLengthEncoded> IsConnected;
    std::vector<ButtonRunLengthEncoded> GetOrigin;
    std::vector<AnalogRunLengthEncoded> TriggerL, TriggerR;
    std::vector<AnalogRunLengthEncoded> AnalogStickX, AnalogStickY;
    std::vector<AnalogRunLengthEncoded> CStickX, CStickY;

    bool VerifyIntegrity() const
    {
        return AllEqual(ButtonRLESum(Start)
            , ButtonRLESum(A)
            , ButtonRLESum(B)
            , ButtonRLESum(X)
            , ButtonRLESum(Y)
            , ButtonRLESum(Z)
            , ButtonRLESum(DPadUp)
            , ButtonRLESum(DPadDown)
            , ButtonRLESum(DPadLeft)
            , ButtonRLESum(DPadRight)
            , ButtonRLESum(L)
            , ButtonRLESum(R)
            , ButtonRLESum(Disc)
            , ButtonRLESum(Reset)
            , ButtonRLESum(IsConnected)
            , ButtonRLESum(GetOrigin)
            , AnalogRLESum(TriggerL)
            , AnalogRLESum(TriggerR)
            , AnalogRLESum(AnalogStickX)
            , AnalogRLESum(AnalogStickY)
            , AnalogRLESum(CStickX)
            , AnalogRLESum(CStickY)
        );
    }

    bool HasNext() const
    {
        // Prioritizing speed. VerifyIntegrity should be called if we want to validate that all arrays are properly set.
        return Start.size() > 0;
    }

    DolphinControllerState PopNext()
    {
        DolphinControllerState Result;

        Result.Start = PopNextButtonState(Start);
        Result.A = PopNextButtonState(A);
        Result.B = PopNextButtonState(B);
        Result.X = PopNextButtonState(X);
        Result.Y = PopNextButtonState(Y);
        Result.Z = PopNextButtonState(Z);
        Result.DPadUp = PopNextButtonState(DPadUp);
        Result.DPadDown = PopNextButtonState(DPadDown);
        Result.DPadLeft = PopNextButtonState(DPadLeft);
        Result.DPadRight = PopNextButtonState(DPadRight);
        Result.L = PopNextButtonState(L);
        Result.R = PopNextButtonState(R);
        Result.Disc = PopNextButtonState(Disc);
        Result.Reset = PopNextButtonState(Reset);
        Result.IsConnected = PopNextButtonState(IsConnected);
        Result.GetOrigin = PopNextButtonState(GetOrigin);
        Result.TriggerL = PopNextAnalogState(TriggerL);
        Result.TriggerR = PopNextAnalogState(TriggerR);
        Result.AnalogStickX = PopNextAnalogState(AnalogStickX);
        Result.AnalogStickY = PopNextAnalogState(AnalogStickY);
        Result.CStickX = PopNextAnalogState(CStickX);
        Result.CStickY = PopNextAnalogState(CStickY);

        return Result;
    }

    void PushNext(DolphinControllerState InputState)
    {
        PushButtonState(Start, InputState.Start);
        PushButtonState(A, InputState.A);
        PushButtonState(B, InputState.B);
        PushButtonState(X, InputState.X);
        PushButtonState(Y, InputState.Y);
        PushButtonState(Z, InputState.Z);
        PushButtonState(DPadUp, InputState.DPadUp);
        PushButtonState(DPadDown, InputState.DPadDown);
        PushButtonState(DPadLeft, InputState.DPadLeft);
        PushButtonState(DPadRight, InputState.DPadRight);
        PushButtonState(L, InputState.L);
        PushButtonState(R, InputState.R);
        PushButtonState(Disc, InputState.Disc);
        PushButtonState(Reset, InputState.Reset);
        PushButtonState(IsConnected, InputState.IsConnected);
        PushButtonState(GetOrigin, InputState.GetOrigin);
        PushAnalogState(TriggerL, InputState.TriggerL);
        PushAnalogState(TriggerR, InputState.TriggerR);
        PushAnalogState(AnalogStickX, InputState.AnalogStickX);
        PushAnalogState(AnalogStickY, InputState.AnalogStickY);
        PushAnalogState(CStickX, InputState.CStickX);
        PushAnalogState(CStickY, InputState.CStickY);
    }

    void Clear()
    {
        Start.clear();
        A.clear();
        B.clear();
        X.clear();
        Y.clear();
        Z.clear();
        DPadUp.clear();
        DPadDown.clear();
        DPadLeft.clear();
        DPadRight.clear();
        L.clear();
        R.clear();
        Disc.clear();
        Reset.clear();
        IsConnected.clear();
        GetOrigin.clear();
        TriggerL.clear();
        TriggerR.clear();
        AnalogStickX.clear();
        AnalogStickY.clear();
        CStickX.clear();
        CStickY.clear();
    }

    int Size() const
    {
        return ButtonRLESum(Start);
    }

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

private:
    void PushButtonState(std::vector<ButtonRunLengthEncoded>& buttonInputs, bool isPressed)
    {
        if (buttonInputs.empty() || buttonInputs.back().Pressed != isPressed)
        {
            buttonInputs.push_back(ButtonRunLengthEncoded(isPressed, 1));
        }
        else
        {
            buttonInputs.back().Length++;
        }
    }

    void PushAnalogState(std::vector<AnalogRunLengthEncoded>& analogInputs, unsigned char value)
    {
        if (analogInputs.empty() || analogInputs.back().Value != value)
        {
            analogInputs.push_back(AnalogRunLengthEncoded(value, 1));
        }
        else
        {
            analogInputs.back().Length++;
        }
    }

    bool PopNextButtonState(std::vector<ButtonRunLengthEncoded>& buttonInputs)
    {
        if (buttonInputs.empty())
        {
            return false;
        }

        bool input = buttonInputs.begin()->Pressed;

        buttonInputs.begin()->Length--;

        if (buttonInputs.begin()->Length <= 0)
        {
            buttonInputs.erase(buttonInputs.begin());
        }

        return input;
    }

    unsigned char PopNextAnalogState(std::vector<AnalogRunLengthEncoded>& analogInputs)
    {
        if (analogInputs.empty())
        {
            return false;
        }

        unsigned char input = analogInputs.begin()->Value;

        analogInputs.begin()->Length--;

        if (analogInputs.begin()->Length <= 0)
        {
            analogInputs.erase(analogInputs.begin());
        }

        return input;
    }

    int ButtonRLESum(const std::vector<ButtonRunLengthEncoded>& buttonInputs) const
    {
        return std::accumulate(buttonInputs.begin(), buttonInputs.end(), 0, [](int sum, const ButtonRunLengthEncoded& curr) { return sum + curr.Length; });
    }

    int AnalogRLESum(const std::vector<AnalogRunLengthEncoded>& analogInputs) const
    {
        return std::accumulate(analogInputs.begin(), analogInputs.end(), 0, [](int sum, const AnalogRunLengthEncoded& curr) { return sum + curr.Length; });
    }
};
