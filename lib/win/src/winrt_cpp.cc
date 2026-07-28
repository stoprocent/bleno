#include "winrt_cpp.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>

#include <winrt/Windows.Devices.Bluetooth.h>

namespace {

std::string NormalizeUuid(std::string uuid) {
    uuid.erase(
        std::remove_if(
            uuid.begin(),
            uuid.end(),
            [](char value) { return value == '-' || value == '{' || value == '}'; }),
        uuid.end());
    std::transform(
        uuid.begin(),
        uuid.end(),
        uuid.begin(),
        [](unsigned char value) { return static_cast<char>(std::tolower(value)); });

    if (!std::all_of(uuid.begin(), uuid.end(), [](unsigned char value) {
        return std::isxdigit(value) != 0;
    })) {
        throw std::invalid_argument("UUID contains non-hexadecimal characters");
    }

    if (uuid.size() == 4) {
        uuid = "0000" + uuid + "00001000800000805f9b34fb";
    } else if (uuid.size() == 8) {
        uuid += "00001000800000805f9b34fb";
    }

    if (uuid.size() != 32) {
        throw std::invalid_argument("UUID must be 16, 32, or 128 bits");
    }

    uuid.insert(8, "-");
    uuid.insert(13, "-");
    uuid.insert(18, "-");
    uuid.insert(23, "-");
    return uuid;
}

}  // namespace

winrt::guid ToGuid(const std::string& uuid) {
    return winrt::guid(winrt::to_hstring(NormalizeUuid(uuid)));
}

winrt::Windows::Storage::Streams::IBuffer ToBuffer(const Data& data) {
    winrt::Windows::Storage::Streams::Buffer buffer(
        static_cast<uint32_t>(data.size()));
    buffer.Length(static_cast<uint32_t>(data.size()));
    if (!data.empty()) {
        std::copy(data.begin(), data.end(), buffer.data());
    }
    return buffer;
}

Data FromBuffer(const winrt::Windows::Storage::Streams::IBuffer& buffer) {
    if (!buffer || buffer.Length() == 0) {
        return {};
    }
    return Data(buffer.data(), buffer.data() + buffer.Length());
}

Data FromNapiValue(const Napi::Value& value) {
    if (value.IsBuffer()) {
        auto buffer = value.As<Napi::Buffer<uint8_t>>();
        return Data(buffer.Data(), buffer.Data() + buffer.Length());
    }
    if (value.IsString()) {
        auto string = value.As<Napi::String>().Utf8Value();
        return Data(string.begin(), string.end());
    }
    return {};
}

std::string ToConnectionId(
    const winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattSession& session) {
    if (!session || !session.DeviceId()) {
        return "unknown";
    }
    return winrt::to_string(session.DeviceId().Id());
}

std::string HResultMessage(const winrt::hresult_error& error) {
    auto message = winrt::to_string(error.message());
    if (!message.empty()) {
        return message;
    }
    return "WinRT error " + std::to_string(error.code().value);
}
