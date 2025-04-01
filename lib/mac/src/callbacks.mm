//
//  callbacks.cc
//  bleno-mac-native
//
//  Created by Georg Vienna on 30.08.18.
//
#include <napi-thread-safe-callback.hpp>
#include "callbacks.h"
#include "napi_objc.h"
#include "objc_cpp.h"

#define _a(val) Napi::String::New(env, convertToBlenoAddress(val))
#define _s(val) Napi::String::New(env, val)
#define _b(val) Napi::Boolean::New(env, val)
#define _n(val) Napi::Number::New(env, val)
#define _u(str) toUuid(env, str)

Napi::Buffer<uint8_t> toBufferFromNSData(Napi::Env& env, const NSData *nsdata) {
    auto data = Data();
    const UInt8* bytes = (UInt8 *)[nsdata bytes];
    data.assign(bytes, bytes + [nsdata length]);
    return toBuffer(env, data);
}

Napi::Buffer<uint8_t> toBuffer(Napi::Env& env, const Data& data) {
    if (data.empty()) {
        return Napi::Buffer<uint8_t>::New(env, 0);
    }
    return Napi::Buffer<uint8_t>::Copy(env, &data[0], data.size());
}

void Emit::Wrap(const Napi::Value& receiver, const Napi::Function& callback) {
    mCallback = std::make_shared<ThreadSafeCallback>(receiver, callback);
}

void Emit::AdvertisingStart(NSError * _Nullable error) {
    mCallback->call([error](Napi::Env env, std::vector<napi_value>& args) {
        const char *cerror = [error.localizedDescription cStringUsingEncoding:NSUTF8StringEncoding];
        args = { _s("advertisingStart"), error ? Napi::Error::New(env, cerror).Value() : env.Null() };
    });
}

void Emit::AdvertisingStop() {
    mCallback->call([](Napi::Env env, std::vector<napi_value>& args) {
        args = { _s("advertisingStop") };
    });
}

void Emit::ServicesSet(NSError * _Nullable error) {
    mCallback->call([error](Napi::Env env, std::vector<napi_value>& args) {
        const char *cerror = [error.localizedDescription cStringUsingEncoding:NSUTF8StringEncoding];
        args = { _s("servicesSet"), error ? Napi::Error::New(env, cerror).Value() : env.Null() };
    });
}

void Emit::StateChange(const std::string& state) {
    mCallback->call([state](Napi::Env env, std::vector<napi_value>& args) {
        args = { _s("stateChange"), _s(state) };
    });
}

void EmitCharacteristic::Wrap(const Napi::Value& receiver, const Napi::Function& callback) {
    mCallback = std::make_shared<ThreadSafeCallback>(receiver, callback);
}

void EmitCharacteristic::ReadRequest(NSUUID *handle, uint16_t offset, std::function<void (uint16_t, NSData *)> completion) {
    mCallback->call([handle, offset, completion](Napi::Env env, std::vector<napi_value>& args) {
        auto callable = [completion](const Napi::CallbackInfo& info){
            completion(info[0].As<Napi::Number>().Int32Value(),
                       napiToData(info[1].As<Napi::Buffer<Byte>>()));
        };
        Napi::Function cb = Napi::Function::New(env, callable);
        args = { _s("readRequest"), _a(handle), _n(offset), cb };
    });
}

void EmitCharacteristic::WriteRequest(NSUUID *handle, NSData *data, uint16_t offset, bool ignoreResponse, std::function<void (uint16_t)> completion) {
    mCallback->call([handle, data, offset, ignoreResponse, completion](Napi::Env env, std::vector<napi_value>& args) {
        auto callable = [completion](const Napi::CallbackInfo& info){
            completion(info[0].As<Napi::Number>().Int32Value());
        };
        Napi::Function cb = Napi::Function::New(env, callable);
        args = { _s("writeRequest"), _a(handle), toBufferFromNSData(env, data), _n(offset), _b(ignoreResponse), cb };
    });
}

void EmitCharacteristic::Subscribe(NSUUID *handle, uint16_t maxValueSize, std::function<void (NSData *)> completion) {
    mCallback->call([handle, maxValueSize, completion](Napi::Env env, std::vector<napi_value>& args) {
        auto callable = [completion](const Napi::CallbackInfo& info){
            completion(napiToData(info[0].As<Napi::Buffer<Byte>>()));
        };
        Napi::Function cb = Napi::Function::New(env, callable);
        args = { _s("subscribe"), _a(handle), _n(maxValueSize), cb };
    });
}

void EmitCharacteristic::Unsubscribe(NSUUID *handle) {
    mCallback->call([handle](Napi::Env env, std::vector<napi_value>& args) {
        args = { _s("unsubscribe"), _a(handle) };
    });
}

void EmitCharacteristic::Notify(NSUUID *handle) {
    mCallback->call([handle](Napi::Env env, std::vector<napi_value>& args) {
        args = { _s("notify"), _a(handle) };
    });
}

void EmitCharacteristic::Indicate(NSUUID *handle) {
    mCallback->call([handle](Napi::Env env, std::vector<napi_value>& args) {
        args = { _s("indicate"), _a(handle) };
    });
}
