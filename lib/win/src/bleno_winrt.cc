#include "bleno_winrt.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include <windows.h>
#include <winrt/Windows.Storage.Streams.h>

#include "winrt_cpp.h"

using winrt::Windows::Devices::Bluetooth::BluetoothError;
using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;

namespace {

constexpr uint16_t ResultSuccess = 0x00;

bool ArrayContains(const Napi::Array& values, const char* expected) {
    for (uint32_t index = 0; index < values.Length(); ++index) {
        if (values.Get(index).ToString().Utf8Value() == expected) {
            return true;
        }
    }
    return false;
}

GattCharacteristicProperties ParseProperties(const Napi::Array& properties) {
    auto result = static_cast<GattCharacteristicProperties>(0);
    for (uint32_t index = 0; index < properties.Length(); ++index) {
        const auto property = properties.Get(index).ToString().Utf8Value();
        if (property == "broadcast") {
            throw std::invalid_argument(
                "Windows GATT server does not support the broadcast characteristic property");
        } else if (property == "read") {
            result |= GattCharacteristicProperties::Read;
        } else if (property == "write") {
            result |= GattCharacteristicProperties::Write;
        } else if (property == "writeWithoutResponse") {
            result |= GattCharacteristicProperties::WriteWithoutResponse;
        } else if (property == "notify") {
            result |= GattCharacteristicProperties::Notify;
        } else if (property == "indicate") {
            result |= GattCharacteristicProperties::Indicate;
        } else if (property == "authenticatedSignedWrites") {
            result |= GattCharacteristicProperties::AuthenticatedSignedWrites;
        } else if (property == "extendedProperties") {
            result |= GattCharacteristicProperties::ExtendedProperties;
        } else {
            throw std::invalid_argument("Unsupported characteristic property: " + property);
        }
    }
    return result;
}

bool HasProperty(GattCharacteristicProperties properties, GattCharacteristicProperties value) {
    return (properties & value) == value;
}

std::string BluetoothErrorMessage(const std::string& operation, BluetoothError error) {
    return operation + " failed with Bluetooth error " +
        std::to_string(static_cast<int32_t>(error));
}

std::vector<ServiceDefinition> ParseServices(const Napi::Array& services) {
    std::vector<ServiceDefinition> result;
    result.reserve(services.Length());

    for (uint32_t serviceIndex = 0; serviceIndex < services.Length(); ++serviceIndex) {
        auto serviceObject = services.Get(serviceIndex).As<Napi::Object>();
        ServiceDefinition service{
            ToGuid(serviceObject.Get("uuid").ToString().Utf8Value()),
            {},
        };

        auto characteristics = serviceObject.Get("characteristics").As<Napi::Array>();
        service.characteristics.reserve(characteristics.Length());
        for (uint32_t characteristicIndex = 0;
             characteristicIndex < characteristics.Length();
             ++characteristicIndex) {
            auto characteristicObject =
                characteristics.Get(characteristicIndex).As<Napi::Object>();
            auto properties =
                characteristicObject.Get("properties").As<Napi::Array>();
            auto secure = characteristicObject.Get("secure").As<Napi::Array>();

            CharacteristicDefinition characteristic{
                ToGuid(characteristicObject.Get("uuid").ToString().Utf8Value()),
                ParseProperties(properties),
                ArrayContains(secure, "read")
                    ? GattProtectionLevel::EncryptionRequired
                    : GattProtectionLevel::Plain,
                ArrayContains(secure, "write") ||
                        ArrayContains(secure, "writeWithoutResponse")
                    ? GattProtectionLevel::EncryptionRequired
                    : GattProtectionLevel::Plain,
            };

            const auto value = characteristicObject.Get("value");
            if (!value.IsNull() && !value.IsUndefined()) {
                characteristic.hasStaticValue = true;
                characteristic.staticValue = FromNapiValue(value);
            }

            auto emit = characteristicObject.Get("emit");
            if (!emit.IsFunction()) {
                throw std::invalid_argument("Characteristic is missing its emit function");
            }
            characteristic.emitter = std::make_shared<EmitCharacteristic>();
            characteristic.emitter->Wrap(
                characteristicObject,
                emit.As<Napi::Function>());

            auto descriptors = characteristicObject.Get("descriptors").As<Napi::Array>();
            for (uint32_t descriptorIndex = 0;
                 descriptorIndex < descriptors.Length();
                 ++descriptorIndex) {
                auto descriptorObject = descriptors.Get(descriptorIndex).As<Napi::Object>();
                const auto descriptorUuid =
                    descriptorObject.Get("uuid").ToString().Utf8Value();
                const auto descriptorValue = FromNapiValue(descriptorObject.Get("value"));

                if (descriptorUuid == "2901") {
                    characteristic.userDescription = std::string(
                        descriptorValue.begin(),
                        descriptorValue.end());
                } else if (descriptorUuid != "2902") {
                    characteristic.descriptors.push_back({
                        ToGuid(descriptorUuid),
                        descriptorValue,
                    });
                }
            }

            service.characteristics.push_back(std::move(characteristic));
        }
        result.push_back(std::move(service));
    }
    return result;
}

winrt::fire_and_forget NotifyClient(
    GattLocalCharacteristic characteristic,
    GattSubscribedClient client,
    Data data) {
    try {
        co_await characteristic.NotifyValueAsync(ToBuffer(data), client);
    } catch (...) {
        // The client may have unsubscribed between the JS callback and this send.
    }
}

}  // namespace

BLEPeripheralManager::BLEPeripheralManager(
    const Napi::Value& receiver,
    const Napi::Function& callback) {
    mEmit.Wrap(receiver, callback);
}

BLEPeripheralManager::~BLEPeripheralManager() {
    Stop();
}

void BLEPeripheralManager::Start() {
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    if (mStopped) {
        return;
    }

    mEmit.Platform("win32");
    mWatcher.Start([this](AdapterState state) {
        std::lock_guard<std::recursive_mutex> callbackLock(mMutex);
        if (mStopped || state == mRadioState) {
            return;
        }
        mRadioState = state;
        mEmit.StateChange(AdapterStateToString(state));
    });
}

void BLEPeripheralManager::StartAdvertising(
    const std::string& name,
    const std::vector<winrt::guid>& serviceUuids) {
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    if (mStopped) {
        return;
    }

    mName = name;
    mAdvertisedServiceUuids = serviceUuids;
    try {
        mAdvertising = true;
        StartProviders();
        mEmit.AdvertisingStart();
    } catch (const winrt::hresult_error& error) {
        mAdvertising = false;
        mEmit.AdvertisingStart(HResultMessage(error));
    } catch (const std::exception& error) {
        mAdvertising = false;
        mEmit.AdvertisingStart(error.what());
    }
}

void BLEPeripheralManager::ReportUnsupportedAdvertising(
    const std::string& feature) {
    mEmit.AdvertisingStart(
        feature + " is not supported by the native Windows binding");
}

void BLEPeripheralManager::StopAdvertising() {
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    for (auto& context : mProviders) {
        try {
            if (context.provider) {
                const auto status = context.provider.AdvertisementStatus();
                if (status != GattServiceProviderAdvertisementStatus::Created &&
                    status != GattServiceProviderAdvertisementStatus::Stopped) {
                    context.provider.StopAdvertising();
                }
            }
        } catch (...) {
        }
    }
    mAdvertising = false;
    if (!mStopped) {
        mEmit.AdvertisingStop();
    }
}

void BLEPeripheralManager::SetServices(
    const std::vector<ServiceDefinition>& services) {
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    if (mStopped) {
        return;
    }

    ClearProviders();
    try {
        for (const auto& service : services) {
            auto providerResult = GattServiceProvider::CreateAsync(service.uuid).get();
            if (providerResult.Error() != BluetoothError::Success ||
                !providerResult.ServiceProvider()) {
                throw std::runtime_error(BluetoothErrorMessage(
                    "Creating GATT service",
                    providerResult.Error()));
            }

            ProviderContext providerContext;
            providerContext.provider = providerResult.ServiceProvider();
            providerContext.advertisementStatusChanged =
                providerContext.provider.AdvertisementStatusChanged(
                    [this](const auto&, const auto& args) {
                        if (args.Status() == GattServiceProviderAdvertisementStatus::Aborted &&
                            !mStopped) {
                            mEmit.AdvertisingStart(BluetoothErrorMessage(
                                "Advertising GATT service",
                                args.Error()));
                        }
                    });
            providerContext.hasAdvertisementStatusChanged = true;
            mProviders.push_back(std::move(providerContext));
            auto& activeProvider = mProviders.back();

            for (const auto& definition : service.characteristics) {
                GattLocalCharacteristicParameters parameters;
                parameters.CharacteristicProperties(definition.properties);
                parameters.ReadProtectionLevel(definition.readProtection);
                parameters.WriteProtectionLevel(definition.writeProtection);
                if (definition.hasStaticValue) {
                    parameters.StaticValue(ToBuffer(definition.staticValue));
                }
                if (!definition.userDescription.empty()) {
                    parameters.UserDescription(
                        winrt::to_hstring(definition.userDescription));
                }

                auto characteristicResult =
                    activeProvider.provider.Service()
                        .CreateCharacteristicAsync(definition.uuid, parameters)
                        .get();
                if (characteristicResult.Error() != BluetoothError::Success ||
                    !characteristicResult.Characteristic()) {
                    throw std::runtime_error(BluetoothErrorMessage(
                        "Creating GATT characteristic",
                        characteristicResult.Error()));
                }

                auto context = std::make_shared<CharacteristicContext>();
                context->characteristic = characteristicResult.Characteristic();
                context->emitter = definition.emitter;
                mCharacteristics.push_back(context);

                if (!definition.hasStaticValue &&
                    HasProperty(definition.properties, GattCharacteristicProperties::Read)) {
                    context->readRequested = context->characteristic.ReadRequested(
                        [this, context](const auto&, const auto& args) {
                            HandleRead(context, args);
                        });
                    context->hasReadRequested = true;
                }

                if (HasProperty(definition.properties, GattCharacteristicProperties::Write) ||
                    HasProperty(
                        definition.properties,
                        GattCharacteristicProperties::WriteWithoutResponse)) {
                    context->writeRequested = context->characteristic.WriteRequested(
                        [this, context](const auto&, const auto& args) {
                            HandleWrite(context, args);
                        });
                    context->hasWriteRequested = true;
                }

                if (HasProperty(definition.properties, GattCharacteristicProperties::Notify) ||
                    HasProperty(definition.properties, GattCharacteristicProperties::Indicate)) {
                    context->subscribedClientsChanged =
                        context->characteristic.SubscribedClientsChanged(
                            [this, context](const auto&, const auto&) {
                                HandleSubscribersChanged(context);
                            });
                    context->hasSubscribedClientsChanged = true;
                }

                for (const auto& descriptor : definition.descriptors) {
                    GattLocalDescriptorParameters descriptorParameters;
                    descriptorParameters.ReadProtectionLevel(GattProtectionLevel::Plain);
                    descriptorParameters.StaticValue(ToBuffer(descriptor.value));
                    auto descriptorResult = context->characteristic
                        .CreateDescriptorAsync(descriptor.uuid, descriptorParameters)
                        .get();
                    if (descriptorResult.Error() != BluetoothError::Success) {
                        throw std::runtime_error(BluetoothErrorMessage(
                            "Creating GATT descriptor",
                            descriptorResult.Error()));
                    }
                }
            }
        }

        if (mAdvertising) {
            StartProviders();
        }
        mEmit.ServicesSet();
    } catch (const winrt::hresult_error& error) {
        ClearProviders();
        mEmit.ServicesSet(HResultMessage(error));
    } catch (const std::exception& error) {
        ClearProviders();
        mEmit.ServicesSet(error.what());
    }
}

void BLEPeripheralManager::StartProviders() {
    GattServiceProviderAdvertisingParameters parameters;
    parameters.IsDiscoverable(true);
    parameters.IsConnectable(true);
    for (auto& context : mProviders) {
        const auto status = context.provider.AdvertisementStatus();
        if (status != GattServiceProviderAdvertisementStatus::Started &&
            status != GattServiceProviderAdvertisementStatus::StartedWithoutAllAdvertisementData) {
            context.provider.StartAdvertising(parameters);
        }
    }
}

void BLEPeripheralManager::ClearProviders() noexcept {
    for (const auto& context : mCharacteristics) {
        for (const auto& subscriber : context->subscribers) {
            try {
                context->emitter->Unsubscribe(subscriber.first);
            } catch (...) {
            }
        }
        UnregisterCharacteristic(context);
    }
    for (auto& context : mProviders) {
        try {
            if (context.provider) {
                if (context.hasAdvertisementStatusChanged) {
                    context.provider.AdvertisementStatusChanged(
                        context.advertisementStatusChanged);
                }
                const auto status = context.provider.AdvertisementStatus();
                if (status != GattServiceProviderAdvertisementStatus::Created &&
                    status != GattServiceProviderAdvertisementStatus::Stopped) {
                    context.provider.StopAdvertising();
                }
            }
        } catch (...) {
        }
    }
    mCharacteristics.clear();
    mProviders.clear();
}

void BLEPeripheralManager::EnsureSession(const GattSession& session) {
    if (!session) {
        return;
    }

    const auto connection = ToConnectionId(session);
    bool isNew = false;
    {
        std::lock_guard<std::recursive_mutex> lock(mMutex);
        if (mStopped || mSessions.find(connection) != mSessions.end()) {
            return;
        }

        auto context = std::make_shared<SessionContext>();
        context->session = session;
        context->statusChanged = session.SessionStatusChanged(
            [this, connection](const auto&, const auto& args) {
                if (args.Status() == GattSessionStatus::Closed) {
                    HandleSessionClosed(connection);
                }
            });
        context->hasStatusChanged = true;
        context->maxPduSizeChanged = session.MaxPduSizeChanged(
            [this](const auto& sender, const auto&) {
                if (!mStopped) {
                    mEmit.MtuChange(sender.MaxPduSize());
                }
            });
        context->hasMaxPduSizeChanged = true;
        mSessions.emplace(connection, std::move(context));
        isNew = true;
    }

    if (isNew) {
        mEmit.Accept(connection);
        mEmit.MtuChange(session.MaxPduSize());
    }
}

void BLEPeripheralManager::HandleSessionClosed(
    const std::string& connection) noexcept {
    try {
        std::lock_guard<std::recursive_mutex> lock(mMutex);
        auto session = mSessions.find(connection);
        if (mStopped || session == mSessions.end()) {
            return;
        }

        if (session->second->hasStatusChanged) {
            session->second->session.SessionStatusChanged(
                session->second->statusChanged);
        }
        if (session->second->hasMaxPduSizeChanged) {
            session->second->session.MaxPduSizeChanged(
                session->second->maxPduSizeChanged);
        }
        mSessions.erase(session);

        for (const auto& context : mCharacteristics) {
            if (context->subscribers.erase(connection) > 0) {
                context->emitter->Unsubscribe(connection);
            }
        }
        mEmit.Disconnect(connection);
    } catch (...) {
    }
}

void BLEPeripheralManager::HandleRead(
    const std::shared_ptr<CharacteristicContext>& context,
    const GattReadRequestedEventArgs& args) {
    auto deferral = args.GetDeferral();
    try {
        auto request = args.GetRequestAsync().get();
        if (!request) {
            deferral.Complete();
            return;
        }

        EnsureSession(args.Session());
        const auto connection = ToConnectionId(args.Session());
        context->emitter->ReadRequest(
            connection,
            static_cast<uint16_t>(request.Offset()),
            [request, deferral](uint16_t result, const Data& data) {
                try {
                    if (result == ResultSuccess) {
                        request.RespondWithValue(ToBuffer(data));
                    } else {
                        request.RespondWithProtocolError(static_cast<uint8_t>(result));
                    }
                } catch (...) {
                }
                deferral.Complete();
            });
    } catch (...) {
        deferral.Complete();
    }
}

void BLEPeripheralManager::HandleWrite(
    const std::shared_ptr<CharacteristicContext>& context,
    const GattWriteRequestedEventArgs& args) {
    auto deferral = args.GetDeferral();
    try {
        auto request = args.GetRequestAsync().get();
        if (!request) {
            deferral.Complete();
            return;
        }

        EnsureSession(args.Session());
        const auto connection = ToConnectionId(args.Session());
        const bool withoutResponse =
            request.Option() != GattWriteOption::WriteWithResponse;
        context->emitter->WriteRequest(
            connection,
            FromBuffer(request.Value()),
            static_cast<uint16_t>(request.Offset()),
            withoutResponse,
            [request, deferral, withoutResponse](uint16_t result) {
                try {
                    if (!withoutResponse) {
                        if (result == ResultSuccess) {
                            request.Respond();
                        } else {
                            request.RespondWithProtocolError(
                                static_cast<uint8_t>(result));
                        }
                    }
                } catch (...) {
                }
                deferral.Complete();
            });
    } catch (...) {
        deferral.Complete();
    }
}

void BLEPeripheralManager::HandleSubscribersChanged(
    const std::shared_ptr<CharacteristicContext>& context) noexcept {
    try {
        std::lock_guard<std::recursive_mutex> lock(mMutex);
        if (mStopped) {
            return;
        }

        std::map<std::string, GattSubscribedClient> current;
        for (const auto& client : context->characteristic.SubscribedClients()) {
            auto session = client.Session();
            EnsureSession(session);
            current.emplace(ToConnectionId(session), client);
        }

        for (const auto& subscriber : context->subscribers) {
            if (current.find(subscriber.first) == current.end()) {
                context->emitter->Unsubscribe(subscriber.first);
            }
        }

        for (const auto& subscriber : current) {
            if (context->subscribers.find(subscriber.first) ==
                context->subscribers.end()) {
                auto characteristic = context->characteristic;
                auto client = subscriber.second;
                context->emitter->Subscribe(
                    subscriber.first,
                    static_cast<uint16_t>(
                        std::max<uint16_t>(client.Session().MaxPduSize(), 3) - 3),
                    [characteristic, client](const Data& data) {
                        NotifyClient(characteristic, client, data);
                    });
            }
        }
        context->subscribers = std::move(current);
    } catch (...) {
    }
}

void BLEPeripheralManager::UnregisterCharacteristic(
    const std::shared_ptr<CharacteristicContext>& context) noexcept {
    if (!context->characteristic) {
        return;
    }
    try {
        if (context->hasReadRequested) {
            context->characteristic.ReadRequested(context->readRequested);
        }
        if (context->hasWriteRequested) {
            context->characteristic.WriteRequested(context->writeRequested);
        }
        if (context->hasSubscribedClientsChanged) {
            context->characteristic.SubscribedClientsChanged(
                context->subscribedClientsChanged);
        }
    } catch (...) {
    }
}

void BLEPeripheralManager::Disconnect(const std::string& connection) {
    std::vector<GattSession> sessions;
    {
        std::lock_guard<std::recursive_mutex> lock(mMutex);
        for (const auto& entry : mSessions) {
            if (connection.empty() || entry.first == connection) {
                sessions.push_back(entry.second->session);
            }
        }
    }
    for (const auto& session : sessions) {
        try {
            session.MaintainConnection(false);
            session.Close();
        } catch (...) {
        }
    }
}

void BLEPeripheralManager::Stop() noexcept {
    std::lock_guard<std::recursive_mutex> lock(mMutex);
    if (mStopped) {
        return;
    }
    mStopped = true;
    mWatcher.Stop();
    ClearProviders();

    for (const auto& entry : mSessions) {
        const auto& context = entry.second;
        try {
            if (context->hasStatusChanged) {
                context->session.SessionStatusChanged(context->statusChanged);
            }
            if (context->hasMaxPduSizeChanged) {
                context->session.MaxPduSizeChanged(context->maxPduSizeChanged);
            }
            context->session.MaintainConnection(false);
            context->session.Close();
        } catch (...) {
        }
    }
    mSessions.clear();
    mAdvertising = false;
}

BlenoWinRT::BlenoWinRT(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<BlenoWinRT>(info) {
}

BlenoWinRT::~BlenoWinRT() {
    if (mPeripheralManager) {
        mPeripheralManager->Stop();
    }
}

BLEPeripheralManager& BlenoWinRT::Manager(const Napi::CallbackInfo& info) {
    if (!mPeripheralManager) {
        throw Napi::Error::New(
            info.Env(),
            "Windows BLE manager is not initialized or has been stopped");
    }
    return *mPeripheralManager;
}

Napi::Value BlenoWinRT::Init(const Napi::CallbackInfo& info) {
    if (mPeripheralManager) {
        return info.Env().Undefined();
    }

    auto receiver = info.This().As<Napi::Object>();
    auto emit = receiver.Get("emit");
    if (!emit.IsFunction()) {
        Napi::TypeError::New(info.Env(), "Binding is missing its emit function")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }

    mPeripheralManager = std::make_unique<BLEPeripheralManager>(
        receiver,
        emit.As<Napi::Function>());
    mPeripheralManager->Start();
    return info.Env().Undefined();
}

Napi::Value BlenoWinRT::Stop(const Napi::CallbackInfo& info) {
    if (mPeripheralManager) {
        mPeripheralManager->Stop();
        mPeripheralManager.reset();
    }
    return info.Env().Undefined();
}

Napi::Value BlenoWinRT::StartAdvertising(const Napi::CallbackInfo& info) {
    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsArray()) {
        Napi::TypeError::New(
            info.Env(),
            "startAdvertising expects a name and an array of service UUIDs")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }

    try {
        std::vector<winrt::guid> serviceUuids;
        auto uuids = info[1].As<Napi::Array>();
        serviceUuids.reserve(uuids.Length());
        for (uint32_t index = 0; index < uuids.Length(); ++index) {
            serviceUuids.push_back(ToGuid(uuids.Get(index).ToString().Utf8Value()));
        }
        Manager(info).StartAdvertising(
            info[0].As<Napi::String>().Utf8Value(),
            serviceUuids);
    } catch (const Napi::Error& error) {
        error.ThrowAsJavaScriptException();
    } catch (const std::exception& error) {
        Napi::TypeError::New(info.Env(), error.what()).ThrowAsJavaScriptException();
    }
    return info.Env().Undefined();
}

Napi::Value BlenoWinRT::StopAdvertising(const Napi::CallbackInfo& info) {
    try {
        Manager(info).StopAdvertising();
    } catch (const Napi::Error& error) {
        error.ThrowAsJavaScriptException();
    }
    return info.Env().Undefined();
}

Napi::Value BlenoWinRT::StartAdvertisingIBeacon(
    const Napi::CallbackInfo& info) {
    try {
        Manager(info).ReportUnsupportedAdvertising("iBeacon advertising");
    } catch (const Napi::Error& error) {
        error.ThrowAsJavaScriptException();
    }
    return info.Env().Undefined();
}

Napi::Value BlenoWinRT::StartAdvertisingWithEIRData(
    const Napi::CallbackInfo& info) {
    try {
        Manager(info).ReportUnsupportedAdvertising("Raw EIR advertising");
    } catch (const Napi::Error& error) {
        error.ThrowAsJavaScriptException();
    }
    return info.Env().Undefined();
}

Napi::Value BlenoWinRT::SetServices(const Napi::CallbackInfo& info) {
    if (info.Length() < 1 || !info[0].IsArray()) {
        Napi::TypeError::New(info.Env(), "setServices expects an array")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }

    try {
        Manager(info).SetServices(ParseServices(info[0].As<Napi::Array>()));
    } catch (const Napi::Error& error) {
        error.ThrowAsJavaScriptException();
    } catch (const std::exception& error) {
        Napi::TypeError::New(info.Env(), error.what()).ThrowAsJavaScriptException();
    }
    return info.Env().Undefined();
}

Napi::Value BlenoWinRT::Disconnect(const Napi::CallbackInfo& info) {
    try {
        std::string connection;
        if (info.Length() > 0 && !info[0].IsNull() && !info[0].IsUndefined()) {
            connection = info[0].ToString().Utf8Value();
        }
        Manager(info).Disconnect(connection);
    } catch (const Napi::Error& error) {
        error.ThrowAsJavaScriptException();
    }
    return info.Env().Undefined();
}

Napi::Value BlenoWinRT::UpdateRssi(const Napi::CallbackInfo& info) {
    Napi::Error::New(
        info.Env(),
        "Peer RSSI is not available from the native Windows GATT server API")
        .ThrowAsJavaScriptException();
    return info.Env().Undefined();
}

Napi::Function BlenoWinRT::GetClass(Napi::Env env) {
    return DefineClass(env, "BlenoWinRT", {
        InstanceMethod("init", &BlenoWinRT::Init),
        InstanceMethod("stop", &BlenoWinRT::Stop),
        InstanceMethod("startAdvertising", &BlenoWinRT::StartAdvertising),
        InstanceMethod("startAdvertisingIBeacon", &BlenoWinRT::StartAdvertisingIBeacon),
        InstanceMethod(
            "startAdvertisingWithEIRData",
            &BlenoWinRT::StartAdvertisingWithEIRData),
        InstanceMethod("stopAdvertising", &BlenoWinRT::StopAdvertising),
        InstanceMethod("setServices", &BlenoWinRT::SetServices),
        InstanceMethod("disconnect", &BlenoWinRT::Disconnect),
        InstanceMethod("updateRssi", &BlenoWinRT::UpdateRssi),
    });
}

Napi::Object InitModule(Napi::Env env, Napi::Object exports) {
    try {
        winrt::init_apartment(winrt::apartment_type::multi_threaded);
    } catch (const winrt::hresult_error& error) {
        if (error.code() != RPC_E_CHANGED_MODE) {
            Napi::Error::New(env, HResultMessage(error)).ThrowAsJavaScriptException();
            return exports;
        }
    }

    exports.Set("BlenoWinRT", BlenoWinRT::GetClass(env));
    return exports;
}

NODE_API_MODULE(addon, InitModule)
