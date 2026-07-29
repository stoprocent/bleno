#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <napi.h>

class ThreadSafeCallback;

using Data = std::vector<uint8_t>;

class Emit {
public:
    void Wrap(const Napi::Value& receiver, const Napi::Function& callback);
    void Platform(const std::string& platform);
    void StateChange(const std::string& state);
    void Warning(const std::string& message);
    void AdvertisingStart(const std::string& error = {});
    void AdvertisingStop();
    void ServicesSet(const std::string& error = {});
    void Accept(const std::string& connection);
    void Disconnect(const std::string& connection);
    void MtuChange(uint16_t mtu);
    void RssiUpdate(int16_t rssi);

private:
    std::shared_ptr<ThreadSafeCallback> mCallback;
};

class EmitCharacteristic {
public:
    void Wrap(const Napi::Value& receiver, const Napi::Function& callback);
    void ReadRequest(
        const std::string& connection,
        uint16_t offset,
        std::function<void(uint16_t, const Data&)> completion);
    void WriteRequest(
        const std::string& connection,
        const Data& data,
        uint16_t offset,
        bool withoutResponse,
        std::function<void(uint16_t)> completion);
    void Subscribe(
        const std::string& connection,
        uint16_t maxValueSize,
        std::function<void(const Data&)> completion);
    void Unsubscribe(const std::string& connection);

private:
    std::shared_ptr<ThreadSafeCallback> mCallback;
};
