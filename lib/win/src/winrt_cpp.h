#pragma once

#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/Windows.Storage.Streams.h>

using winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristicProperties;
using winrt::Windows::Storage::Streams::IBuffer;

IBuffer stringToIBuffer(const std::string& str);
std::string ws2s(const wchar_t* wstr);
std::string formatBluetoothAddress(unsigned long long BluetoothAddress);
std::string formatBluetoothUuid(unsigned long long BluetoothAddress);
std::string toStr(winrt::guid uuid);
std::vector<std::string> toPropertyArray(GattCharacteristicProperties& properties);
