//
//  callbacks.cc
//  noble-mac-native
//
//  Created by Georg Vienna on 30.08.18.
//
#include "callbacks.h"

#include <napi-thread-safe-callback.hpp>

#define _s(val) Napi::String::New(env, val)
#define _b(val) Napi::Boolean::New(env, val)
#define _n(val) Napi::Number::New(env, val)
#define _u(str) toUuid(env, str)

using Data = std::vector<uint8_t>;

Napi::String toUuid(Napi::Env& env, const std::string& uuid)
{
    std::string str(uuid);
    str.erase(std::remove(str.begin(), str.end(), '-'), str.end());
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return _s(str);
}

Napi::Buffer<uint8_t> toBuffer(Napi::Env& env, const Data& data)
{
    if (data.empty())
    {
        return Napi::Buffer<uint8_t>::New(env, 0);
    }
    return Napi::Buffer<uint8_t>::Copy(env, &data[0], data.size());
}

Napi::Array toUuidArray(Napi::Env& env, const std::vector<std::string>& data)
{
    if (data.empty())
    {
        return Napi::Array::New(env);
    }
    auto arr = Napi::Array::New(env, data.size());
    for (size_t i = 0; i < data.size(); i++)
    {
        arr.Set(i, _u(data[i]));
    }
    return arr;
}

Napi::Array toArray(Napi::Env& env, const std::vector<std::string>& data)
{
    if (data.empty())
    {
        return Napi::Array::New(env);
    }
    auto arr = Napi::Array::New(env, data.size());
    for (size_t i = 0; i < data.size(); i++)
    {
        arr.Set(i, _s(data[i]));
    }
    return arr;
}

void Emit::Wrap(const Napi::Value& receiver, const Napi::Function& callback)
{
    mCallback = std::make_shared<ThreadSafeCallback>(receiver, callback);
}

void Emit::RadioState(const std::string& state)
{
    mCallback->call([state](Napi::Env env, std::vector<napi_value>& args) {
        // emit('stateChange', state);
        args = { _s("stateChange"), _s(state) };
    });
}

void Emit::AdvertisingStart(const std::string& error)
{
    mCallback->call([error](Napi::Env env, std::vector<napi_value>& args) {
        // emit('stateChange', state);
        args = { _s("advertisingStart"), _s(error) };
    });
}
