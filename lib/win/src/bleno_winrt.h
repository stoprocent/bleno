#pragma once

#include <napi.h>
#include <memory>

#include <windows.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/base.h>  // For winrt::to_string

#include "callbacks.h"
#include "radio_watcher.h"

// Add these includes
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Core.h>

using namespace winrt;
using namespace Windows::Devices::Bluetooth;
using namespace Windows::Devices::Bluetooth::Advertisement;
using namespace Windows::Devices::Bluetooth::GenericAttributeProfile;

// Forward declaration
class BLEPeripheralManager;

class BlenoWinRT : public Napi::ObjectWrap<BlenoWinRT> {
private:
    BLEPeripheralManager* peripheralManager;

public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    BlenoWinRT(const Napi::CallbackInfo& info);

    // Node.js accessible methods
    Napi::Value Init(const Napi::CallbackInfo& info);
    Napi::Value StartAdvertising(const Napi::CallbackInfo& info);
    Napi::Value StopAdvertising(const Napi::CallbackInfo& info);
    Napi::Value SetServices(const Napi::CallbackInfo& info);
    Napi::Value Disconnect(const Napi::CallbackInfo& info);
    Napi::Value UpdateRssi(const Napi::CallbackInfo& info);
    Napi::Value Stop(const Napi::CallbackInfo& info);
};

class BLEPeripheralManager {
public:
    BLEPeripheralManager(const Napi::Value& receiver, const Napi::Function& callback);
    void Start();
    void StartAdvertising(const std::string& name, const std::vector<GUID>& serviceUuids);
    void StopAdvertising();
    void SetServices(const std::vector<GattLocalService>& newServices);
    void Disconnect();
    void UpdateRssi();
    void Stop();

    // Callback setters
    void SetStateChangeCallback(std::function<void(const char*)> cb);
    void SetAdvertisingStartCallback(std::function<void(const char*)> cb);
    void SetAdvertisingStopCallback(std::function<void()> cb);
    void SetServicesSetCallback(std::function<void(const char*)> cb);

private:
    BluetoothLEAdvertisementPublisher mAdvertiser{ nullptr };
    winrt::Windows::System::DispatcherQueue dispatcherQueue{ nullptr };
    AdapterState mRadioState;
    RadioWatcher mWatcher;
    Emit mEmit;
    std::vector<GattServiceProvider> mProviders;
    
    bool isAdvertising;
    
    // Callback handlers
    std::function<void(const char*)> stateChangeCallback;
    std::function<void(const char*)> advertisingStartCallback;
    std::function<void()> advertisingStopCallback;
    std::function<void(const char*)> servicesSetCallback;

    void OnRadio(Radio& radio);

    void EmitStateChange(const char* state);
    void EmitAdvertisingStart(const char* error);
    void EmitAdvertisingStop();
    void EmitServicesSet(const char* error);
}; 