#pragma once

#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <napi.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>

#include "callbacks.h"
#include "radio_watcher.h"

struct DescriptorDefinition {
    winrt::guid uuid;
    Data value;
};

struct CharacteristicDefinition {
    winrt::guid uuid;
    winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristicProperties properties;
    winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattProtectionLevel readProtection;
    winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattProtectionLevel writeProtection;
    bool hasStaticValue{ false };
    Data staticValue;
    std::string userDescription;
    std::vector<DescriptorDefinition> descriptors;
    std::shared_ptr<EmitCharacteristic> emitter;
};

struct ServiceDefinition {
    winrt::guid uuid;
    std::vector<CharacteristicDefinition> characteristics;
};

class BLEPeripheralManager {
public:
    BLEPeripheralManager(const Napi::Value& receiver, const Napi::Function& callback);
    ~BLEPeripheralManager();

    void Start();
    void StartAdvertising(
        const std::string& name,
        const std::vector<winrt::guid>& serviceUuids);
    void ReportUnsupportedAdvertising(const std::string& feature);
    void StopAdvertising();
    void SetServices(const std::vector<ServiceDefinition>& services);
    void Disconnect(const std::string& connection = {});
    void Stop() noexcept;

private:
    using GattLocalCharacteristic =
        winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattLocalCharacteristic;
    using GattServiceProvider =
        winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattServiceProvider;
    using GattSession =
        winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattSession;
    using GattSubscribedClient =
        winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattSubscribedClient;

    struct CharacteristicContext {
        GattLocalCharacteristic characteristic{ nullptr };
        std::shared_ptr<EmitCharacteristic> emitter;
        std::map<std::string, GattSubscribedClient> subscribers;
        winrt::event_token readRequested;
        winrt::event_token writeRequested;
        winrt::event_token subscribedClientsChanged;
        bool hasReadRequested{ false };
        bool hasWriteRequested{ false };
        bool hasSubscribedClientsChanged{ false };
    };

    struct ProviderContext {
        GattServiceProvider provider{ nullptr };
        winrt::event_token advertisementStatusChanged;
        bool hasAdvertisementStatusChanged{ false };
    };

    struct SessionContext {
        GattSession session{ nullptr };
        winrt::event_token statusChanged;
        winrt::event_token maxPduSizeChanged;
        bool hasStatusChanged{ false };
        bool hasMaxPduSizeChanged{ false };
    };

    void StartProviders();
    void ClearProviders() noexcept;
    void EnsureSession(const GattSession& session);
    void HandleSessionClosed(const std::string& connection) noexcept;
    void HandleRead(
        const std::shared_ptr<CharacteristicContext>& context,
        const winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattReadRequestedEventArgs& args);
    void HandleWrite(
        const std::shared_ptr<CharacteristicContext>& context,
        const winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattWriteRequestedEventArgs& args);
    void HandleSubscribersChanged(
        const std::shared_ptr<CharacteristicContext>& context) noexcept;
    static void UnregisterCharacteristic(
        const std::shared_ptr<CharacteristicContext>& context) noexcept;

    std::recursive_mutex mMutex;
    bool mAdvertising{ false };
    std::atomic<bool> mStopped{ false };
    AdapterState mRadioState{ AdapterState::Initial };
    std::string mName;
    std::vector<winrt::guid> mAdvertisedServiceUuids;
    std::vector<ProviderContext> mProviders;
    std::vector<std::shared_ptr<CharacteristicContext>> mCharacteristics;
    std::map<std::string, std::shared_ptr<SessionContext>> mSessions;
    RadioWatcher mWatcher;
    Emit mEmit;
};

class BlenoWinRT : public Napi::ObjectWrap<BlenoWinRT> {
public:
    explicit BlenoWinRT(const Napi::CallbackInfo& info);
    ~BlenoWinRT() override;

    Napi::Value Init(const Napi::CallbackInfo& info);
    Napi::Value Stop(const Napi::CallbackInfo& info);
    Napi::Value StartAdvertising(const Napi::CallbackInfo& info);
    Napi::Value StartAdvertisingIBeacon(const Napi::CallbackInfo& info);
    Napi::Value StartAdvertisingWithEIRData(const Napi::CallbackInfo& info);
    Napi::Value StopAdvertising(const Napi::CallbackInfo& info);
    Napi::Value SetServices(const Napi::CallbackInfo& info);
    Napi::Value Disconnect(const Napi::CallbackInfo& info);
    Napi::Value UpdateRssi(const Napi::CallbackInfo& info);

    static Napi::Function GetClass(Napi::Env env);

private:
    BLEPeripheralManager& Manager(const Napi::CallbackInfo& info);
    std::unique_ptr<BLEPeripheralManager> mPeripheralManager;
};
