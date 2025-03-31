#pragma once

#include <napi.h>
#include "peripheral.h"

#import <Foundation/Foundation.h>

class ThreadSafeCallback;

NS_ASSUME_NONNULL_BEGIN
class Emit {
public:
    void Wrap(const Napi::Value& receiver, const Napi::Function& callback);
    void AdvertisingStart(NSError * _Nullable error);
    void ServicesSet(NSError * _Nullable error);
    void StateChange(const std::string& state);
protected:
    std::shared_ptr<ThreadSafeCallback> mCallback;
};

class EmitCharacteristic {
public:
    void Wrap(const Napi::Value& receiver, const Napi::Function& callback);
    void ReadRequest(uint16_t offset, std::function<void (uint16_t, NSData *)> completion);
    void WriteRequest(NSData *data, uint16_t offset, bool ignoreResponse, std::function<void (uint16_t)> completion);
    void Subscribe(uint16_t maxValueSize, std::function<void (NSData *)> completion);
    void Unsubscribe();
    void Notify();
    void Indicate();
protected:
    std::shared_ptr<ThreadSafeCallback> mCallback;

private:
    Napi::Buffer<uint8_t> toBuffer(Napi::Env& env, const Data& data);
};

Napi::Buffer<uint8_t> toBufferFromNSData(Napi::Env& env, const NSData *data);
Napi::Buffer<uint8_t> toBuffer(Napi::Env& env, const Data& data);
NS_ASSUME_NONNULL_END
