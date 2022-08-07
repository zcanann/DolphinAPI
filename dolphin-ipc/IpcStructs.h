#pragma once

#include "external/cereal/cereal.hpp"

enum class DolphinSlot
{
    SlotA,
    SlotB,
    SP1,
};

enum class DolphinDataType
{
    Int8,
    UInt8,
    Int16,
    UInt16,
    Int32,
    UInt32,
    Int64,
    UInt64,
    Float,
    Double,
    ArrayOfBytes,
    ArrayOfUBytes,
    String,
};

union DolphinValue
{
    DolphinValue() : _valueInt8(0) { }
    ~DolphinValue() { }

    void CopyFrom(const DolphinValue& other, DolphinDataType dataType)
    {
        switch (dataType)
        {
            case DolphinDataType::Int8: _valueInt8 = other._valueInt8; break;
            case DolphinDataType::UInt8: _valueUInt8 = other._valueUInt8; break;
            case DolphinDataType::Int16: _valueInt16 = other._valueInt16; break;
            case DolphinDataType::UInt16: _valueUInt16 = other._valueUInt16; break;
            case DolphinDataType::Int32: _valueInt32 = other._valueInt32; break;
            case DolphinDataType::UInt32: _valueUInt32 = other._valueUInt32; break;
            case DolphinDataType::Int64: _valueInt64 = other._valueInt64; break;
            case DolphinDataType::UInt64: _valueUInt64 = other._valueUInt64; break;
            case DolphinDataType::Float: _valueFloat = other._valueFloat; break;
            case DolphinDataType::Double: _valueDouble = other._valueDouble; break;
            case DolphinDataType::ArrayOfBytes: _valueArrayOfBytes = other._valueArrayOfBytes; break;
            case DolphinDataType::ArrayOfUBytes: _valueArrayOfUBytes = other._valueArrayOfUBytes; break;
            case DolphinDataType::String: _valueString = other._valueString; break;
        }
    }

    template <class Archive>
    void Serialize(Archive& ar, DolphinDataType dataType)
    {
        switch (dataType)
        {
            case DolphinDataType::Int8: ar(_valueInt8); break;
            case DolphinDataType::UInt8: ar(_valueUInt8); break;
            case DolphinDataType::Int16: ar(_valueInt16); break;
            case DolphinDataType::UInt16: ar(_valueUInt16); break;
            case DolphinDataType::Int32: ar(_valueInt32); break;
            case DolphinDataType::UInt32: ar(_valueUInt32); break;
            case DolphinDataType::Int64: ar(_valueInt64); break;
            case DolphinDataType::UInt64: ar(_valueUInt64); break;
            case DolphinDataType::Float: ar(_valueFloat); break;
            case DolphinDataType::Double: ar(_valueDouble); break;
            case DolphinDataType::ArrayOfBytes: ar(_valueArrayOfBytes); break;
            case DolphinDataType::ArrayOfUBytes: ar(_valueArrayOfUBytes); break;
            case DolphinDataType::String: ar(_valueString); break;
        }
    }

    signed char _valueInt8;
    unsigned char _valueUInt8;
    signed short _valueInt16;
    unsigned short _valueUInt16;
    signed int _valueInt32;
    unsigned int _valueUInt32;
    signed long long _valueInt64;
    unsigned long long _valueUInt64;
    float _valueFloat;
    double _valueDouble;
    std::vector<signed char> _valueArrayOfBytes;
    std::vector<unsigned char> _valueArrayOfUBytes;
    std::string _valueString;
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
