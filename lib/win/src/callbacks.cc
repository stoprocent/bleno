#include "callbacks.h"

#include "ThreadSafeCallback.h"

namespace {

Napi::Buffer<uint8_t> ToBuffer(Napi::Env env, const Data& data) {
    if (data.empty()) {
        return Napi::Buffer<uint8_t>::New(env, 0);
    }
    return Napi::Buffer<uint8_t>::Copy(env, data.data(), data.size());
}

Data FromBuffer(const Napi::Value& value) {
    if (!value.IsBuffer()) {
        return {};
    }

    auto buffer = value.As<Napi::Buffer<uint8_t>>();
    return Data(buffer.Data(), buffer.Data() + buffer.Length());
}

}  // namespace

void Emit::Wrap(const Napi::Value& receiver, const Napi::Function& callback) {
    mCallback = std::make_shared<ThreadSafeCallback>(receiver, callback);
}

void Emit::Platform(const std::string& platform) {
    mCallback->call([platform](Napi::Env env, std::vector<napi_value>& args) {
        args = {
            Napi::String::New(env, "platform"),
            Napi::String::New(env, platform),
        };
    });
}

void Emit::StateChange(const std::string& state) {
    mCallback->call([state](Napi::Env env, std::vector<napi_value>& args) {
        args = {
            Napi::String::New(env, "stateChange"),
            Napi::String::New(env, state),
        };
    });
}

void Emit::AdvertisingStart(const std::string& error) {
    mCallback->call([error](Napi::Env env, std::vector<napi_value>& args) {
        args = {
            Napi::String::New(env, "advertisingStart"),
            error.empty()
                ? env.Null()
                : Napi::Error::New(env, error).Value(),
        };
    });
}

void Emit::AdvertisingStop() {
    mCallback->call([](Napi::Env env, std::vector<napi_value>& args) {
        args = { Napi::String::New(env, "advertisingStop") };
    });
}

void Emit::ServicesSet(const std::string& error) {
    mCallback->call([error](Napi::Env env, std::vector<napi_value>& args) {
        args = {
            Napi::String::New(env, "servicesSet"),
            error.empty()
                ? env.Null()
                : Napi::Error::New(env, error).Value(),
        };
    });
}

void Emit::Accept(const std::string& connection) {
    mCallback->call([connection](Napi::Env env, std::vector<napi_value>& args) {
        auto id = Napi::String::New(env, connection);
        args = { Napi::String::New(env, "accept"), id, id };
    });
}

void Emit::Disconnect(const std::string& connection) {
    mCallback->call([connection](Napi::Env env, std::vector<napi_value>& args) {
        auto id = Napi::String::New(env, connection);
        args = { Napi::String::New(env, "disconnect"), id, id };
    });
}

void Emit::MtuChange(uint16_t mtu) {
    mCallback->call([mtu](Napi::Env env, std::vector<napi_value>& args) {
        args = {
            Napi::String::New(env, "mtuChange"),
            Napi::Number::New(env, mtu),
        };
    });
}

void Emit::RssiUpdate(int16_t rssi) {
    mCallback->call([rssi](Napi::Env env, std::vector<napi_value>& args) {
        args = {
            Napi::String::New(env, "rssiUpdate"),
            Napi::Number::New(env, rssi),
        };
    });
}

void EmitCharacteristic::Wrap(
    const Napi::Value& receiver,
    const Napi::Function& callback) {
    mCallback = std::make_shared<ThreadSafeCallback>(receiver, callback);
}

void EmitCharacteristic::ReadRequest(
    const std::string& connection,
    uint16_t offset,
    std::function<void(uint16_t, const Data&)> completion) {
    mCallback->call(
        [connection, offset, completion](
            Napi::Env env,
            std::vector<napi_value>& args) {
            auto callback = Napi::Function::New(
                env,
                [completion](const Napi::CallbackInfo& info) {
                    const auto result = info.Length() > 0 && info[0].IsNumber()
                        ? info[0].As<Napi::Number>().Uint32Value()
                        : 0x0e;
                    const auto data = info.Length() > 1
                        ? FromBuffer(info[1])
                        : Data{};
                    completion(static_cast<uint16_t>(result), data);
                });

            args = {
                Napi::String::New(env, "readRequest"),
                Napi::String::New(env, connection),
                Napi::Number::New(env, offset),
                callback,
            };
        });
}

void EmitCharacteristic::WriteRequest(
    const std::string& connection,
    const Data& data,
    uint16_t offset,
    bool withoutResponse,
    std::function<void(uint16_t)> completion) {
    mCallback->call(
        [connection, data, offset, withoutResponse, completion](
            Napi::Env env,
            std::vector<napi_value>& args) {
            auto callback = Napi::Function::New(
                env,
                [completion](const Napi::CallbackInfo& info) {
                    const auto result = info.Length() > 0 && info[0].IsNumber()
                        ? info[0].As<Napi::Number>().Uint32Value()
                        : 0x0e;
                    completion(static_cast<uint16_t>(result));
                });

            args = {
                Napi::String::New(env, "writeRequest"),
                Napi::String::New(env, connection),
                ToBuffer(env, data),
                Napi::Number::New(env, offset),
                Napi::Boolean::New(env, withoutResponse),
                callback,
            };
        });
}

void EmitCharacteristic::Subscribe(
    const std::string& connection,
    uint16_t maxValueSize,
    std::function<void(const Data&)> completion) {
    mCallback->call(
        [connection, maxValueSize, completion](
            Napi::Env env,
            std::vector<napi_value>& args) {
            auto callback = Napi::Function::New(
                env,
                [completion](const Napi::CallbackInfo& info) {
                    if (info.Length() > 0 && info[0].IsBuffer()) {
                        completion(FromBuffer(info[0]));
                    }
                });

            args = {
                Napi::String::New(env, "subscribe"),
                Napi::String::New(env, connection),
                Napi::Number::New(env, maxValueSize),
                callback,
            };
        });
}

void EmitCharacteristic::Unsubscribe(const std::string& connection) {
    mCallback->call([connection](Napi::Env env, std::vector<napi_value>& args) {
        args = {
            Napi::String::New(env, "unsubscribe"),
            Napi::String::New(env, connection),
        };
    });
}
