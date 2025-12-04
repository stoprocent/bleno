#pragma once

#include <napi.h>

#import <Foundation/Foundation.h>

class ThreadSafeCallback;
using Data = std::vector<uint8_t>;

NS_ASSUME_NONNULL_BEGIN
class Emit {
public:
    void Wrap(const Napi::Value& receiver, const Napi::Function& callback);
    void AdvertisingStart(NSError * _Nullable error);
    void AdvertisingStop();
    void ServicesSet(NSError * _Nullable error);
    void StateChange(const std::string& state);
    void Accept(NSUUID *centralUuid);
    void Disconnect(NSUUID *centralUuid);
protected:
    std::shared_ptr<ThreadSafeCallback> mCallback;
};

class EmitCharacteristic {
public:
    void Wrap(const Napi::Value& receiver, const Napi::Function& callback);
    void ReadRequest(NSUUID *handle, uint16_t offset, std::function<void (uint16_t, NSData *)> completion);
    void WriteRequest(NSUUID *handle, NSData *data, uint16_t offset, bool ignoreResponse, std::function<void (uint16_t)> completion);
    void Subscribe(NSUUID *handle, uint16_t maxValueSize, std::function<void (NSData *)> completion);
    void Unsubscribe(NSUUID *handle);
    void Notify(NSUUID *handle);
    void Indicate(NSUUID *handle);
protected:
    std::shared_ptr<ThreadSafeCallback> mCallback;

private:
    Napi::Buffer<uint8_t> toBuffer(Napi::Env& env, const Data& data);
};

Napi::Buffer<uint8_t> toBufferFromNSData(Napi::Env& env, const NSData *data);
Napi::Buffer<uint8_t> toBuffer(Napi::Env& env, const Data& data);
NS_ASSUME_NONNULL_END
