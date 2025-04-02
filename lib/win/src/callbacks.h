#pragma once

#include <napi.h>


class ThreadSafeCallback;

class Emit
{
public:
    // clang-format off
    void Wrap(const Napi::Value& receiver, const Napi::Function& callback);
    void RadioState(const std::string& status);
    void AdvertisingStart(const std::string& error);
    // clang-format on
protected:
    std::shared_ptr<ThreadSafeCallback> mCallback;
};
