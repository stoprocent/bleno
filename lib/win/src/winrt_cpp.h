#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <napi.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/Windows.Storage.Streams.h>

using Data = std::vector<uint8_t>;

winrt::guid ToGuid(const std::string& uuid);
winrt::Windows::Storage::Streams::IBuffer ToBuffer(const Data& data);
Data FromBuffer(const winrt::Windows::Storage::Streams::IBuffer& buffer);
Data FromNapiValue(const Napi::Value& value);
std::string ToConnectionId(
    const winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattSession& session);
std::string HResultMessage(const winrt::hresult_error& error);
