#include "bleno_winrt.h"
#include <string>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.Streams.h>
#include "winrt_cpp.h"
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Core.h>

using winrt::Windows::Devices::Bluetooth::BluetoothCacheMode;
using winrt::Windows::Devices::Bluetooth::BluetoothConnectionStatus;
using winrt::Windows::Devices::Bluetooth::BluetoothLEDevice;
using winrt::Windows::Storage::Streams::DataReader;
using winrt::Windows::Storage::Streams::DataWriter;
using winrt::Windows::Storage::Streams::IBuffer;

// BLEPeripheralManager implementation
BLEPeripheralManager::BLEPeripheralManager(const Napi::Value& receiver, const Napi::Function& callback) {
    isAdvertising = false;
    mEmit.Wrap(receiver, callback);
    mRadioState = AdapterState::Initial;

    auto onRadio = std::bind(&BLEPeripheralManager::OnRadio, this, std::placeholders::_1);
    mWatcher.Start(onRadio);
}

void BLEPeripheralManager::OnRadio(Radio& radio)
{
    auto state = AdapterState::Unsupported;
    if (radio)
    {
        state = (AdapterState)radio.State();
    }
    if (state != mRadioState)
    {
        mRadioState = state;
        mEmit.RadioState(adapterStateToString(state));
    }
}

void BLEPeripheralManager::Start() {
    // init_apartment();
    EmitStateChange("poweredOn");
}

void BLEPeripheralManager::StartAdvertising(const std::string& name, const std::vector<GUID>& serviceUuids) {
    if (isAdvertising) {
        return;
    }

    mAdvertiser = BluetoothLEAdvertisementPublisher();
    // mAdvertiser.Advertisement().LocalName(std::wstring(L"bleson"));
    
    BluetoothLEManufacturerData manufacturerData = BluetoothLEManufacturerData();
	manufacturerData.CompanyId(0x012E);
	manufacturerData.Data(stringToIBuffer("Jebac biede"));
    mAdvertiser.Advertisement().ManufacturerData().Append(manufacturerData);


    // mAdvertiser.Advertisement().Flags(BluetoothLEAdvertisementFlags::GeneralDiscoverableMode); //set adv flags  


    // // Get the service UUIDs vector
    // auto uuids = advertisement.ServiceUuids();
    
    // // Add each UUID to the WinRT vector
    // for (const auto& uuid : serviceUuids) {
    //     winrt::guid winrtUuid{ uuid.Data1, uuid.Data2, uuid.Data3,
    //         { uuid.Data4[0], uuid.Data4[1], uuid.Data4[2], uuid.Data4[3],
    //           uuid.Data4[4], uuid.Data4[5], uuid.Data4[6], uuid.Data4[7] } };
    //     uuids.Append(winrtUuid);
    // }

    try {
        // mAdvertiser.Start();
        isAdvertising = true;
        mEmit.AdvertisingStart("");
    } catch (const winrt::hresult_error& ex) {
        winrt::hresult code = ex.code();
        std::string error = winrt::to_string(ex.message());
        mEmit.AdvertisingStart(error.c_str());
    }
}

void BLEPeripheralManager::StopAdvertising() {
    if (!isAdvertising) {
        return;
    }

    //mAdvertiser.Stop();
    isAdvertising = false;
    EmitAdvertisingStop();
}

void BLEPeripheralManager::SetServices(const std::vector<GattLocalService>& newServices) {
    try {
        winrt::guid guid2 = guid(L"A1E8F5B1-696B-4E4C-87C6-69DFE0B0093B");
        winrt::guid guid4 = guid(L"A1E8F5B2-696B-4E4C-87C6-69DFE0B0093B");
        auto result = GattServiceProvider::CreateAsync(guid2).get();
        auto provider = result.ServiceProvider();
        GattLocalCharacteristicParameters char_params = GattLocalCharacteristicParameters();
        char_params.CharacteristicProperties(
            GattCharacteristicProperties::Read | 
            GattCharacteristicProperties::Notify |
            GattCharacteristicProperties::Write | 
            GattCharacteristicProperties::WriteWithoutResponse
        );
        
        auto characteristicRes = provider.Service().CreateCharacteristicAsync(guid4, char_params).get();
        auto characteristic = characteristicRes.Characteristic();
        
        characteristic.SubscribedClientsChanged([this](GattLocalCharacteristic characteristic, winrt::Windows::Foundation::IInspectable args) {
            printf("Subscribed clients changed");
            
            for (auto client : characteristic.SubscribedClients()) {
                printf("Client: %ls", client.Session().DeviceId().Id().c_str());
                auto str = client.Session().DeviceId().Id().c_str();
                
                characteristic.NotifyValueAsync(stringToIBuffer(std::string()));
            }
        });

        // auto controller = winrt::Windows::System::DispatcherQueueController::CreateOnDedicatedThread();
        // dispatcherQueue = controller.DispatcherQueue();

        // winrt::fire_and_forget([this, characteristic]() -> winrt::fire_and_forget {
        //     while (true) {
        //         try {
        //             // Switch to dispatcher queue thread
        //             co_await winrt::resume_foreground(dispatcherQueue);
        //             printf("i: %d\n", characteristic.SubscribedClients().Size());
        //             co_await characteristic.NotifyValueAsync(stringToIBuffer("Jebac biede"));
                    
        //             // Switch back to background for waiting
        //             co_await winrt::resume_background();
        //             co_await winrt::resume_after(std::chrono::seconds(1));
        //         }
        //         catch (const winrt::hresult_error&) {
        //             co_return;
        //         }
        //     }
        // }());

        characteristic.ReadRequested([this](GattLocalCharacteristic characteristic, GattReadRequestedEventArgs args) {
            auto request = args.GetRequestAsync().get();
            request.RespondWithValue(stringToIBuffer("Jebac biede"));
            printf("reading");
        });

        characteristic.WriteRequested([this](GattLocalCharacteristic characteristic, GattWriteRequestedEventArgs args) {
            auto result = args.GetRequestAsync().get();
            auto byteData = reinterpret_cast<const uint8_t*>(result.Value().data());
            auto bufferLength = result.Value().Length();
            
            std::string utf8String = winrt::to_string(args.Session().DeviceId().Id());
            printf("Printf: %s\n", utf8String.c_str());
            
            args.Session().SessionStatusChanged([this](GattSession session, GattSessionStatusChangedEventArgs eventArgs) {
                
                printf("Sessions status %d Closed", GattSessionStatus::Closed() == eventArgs.Status());
                // printf("Sessions status %d Active", GattSessionStatus::Active() == eventArgs.Status());
            });
            // Print the buffer length
            printf("Buffer length: %u bytes - offset: %u - max PDU: %u\n", bufferLength, result.Offset(), args.Session().MaxPduSize());
            
            // Print each byte as a hex value
            printf("Hex data: ");
            for (uint32_t i = 0; i < bufferLength; i++) {
                printf("%02X ", byteData[i]);
                
                // Optional: Add a newline every 16 bytes for readability
                if ((i + 1) % 16 == 0 && i < bufferLength - 1) {
                    printf("\n");
                }
            }
            printf("\n");
            
        });

        auto parameters = GattServiceProviderAdvertisingParameters();
        parameters.IsDiscoverable(true);
        parameters.IsConnectable(true);
        
        // auto statusChanged = 
        provider.AdvertisementStatusChanged([this](GattServiceProvider serviceProvider, GattServiceProviderAdvertisementStatusChangedEventArgs eventArgs) {
            auto error = eventArgs.Error();
            auto status = eventArgs.Status();
            printf("Error %d\n", error);
            printf("STATUS Aborted %d\n", serviceProvider.AdvertisementStatus() == GattServiceProviderAdvertisementStatus::Aborted);
            printf("STATUS Created %d\n", serviceProvider.AdvertisementStatus() == GattServiceProviderAdvertisementStatus::Created);
            printf("STATUS Started %d\n", serviceProvider.AdvertisementStatus() == GattServiceProviderAdvertisementStatus::Started);
            printf("STATUS StartedWithoutAllAdvertisementData %d\n", serviceProvider.AdvertisementStatus() == GattServiceProviderAdvertisementStatus::StartedWithoutAllAdvertisementData);
            printf("STATUS Stopped %d\n", serviceProvider.AdvertisementStatus() == GattServiceProviderAdvertisementStatus::Stopped);
        });

        provider.StartAdvertising(parameters);
        mProviders.push_back(provider);
        

        EmitServicesSet("");
    } catch (const winrt::hresult_error& ex) {
        std::string error = winrt::to_string(ex.message());
        EmitServicesSet(error.c_str());
    }
}

void BLEPeripheralManager::Disconnect() {
    // Implementation for disconnecting
}

void BLEPeripheralManager::UpdateRssi() {
    // Implementation for RSSI updates
}

void BLEPeripheralManager::Stop() {
    // Implementation for stopping
    mAdvertiser.Stop();
    for (auto provider : mProviders) {
        provider.StopAdvertising();
    }
}

// Callback setters
void BLEPeripheralManager::SetStateChangeCallback(std::function<void(const char*)> cb) {
    stateChangeCallback = cb;
}

void BLEPeripheralManager::SetAdvertisingStartCallback(std::function<void(const char*)> cb) {
    advertisingStartCallback = cb;
}

void BLEPeripheralManager::SetAdvertisingStopCallback(std::function<void()> cb) {
    advertisingStopCallback = cb;
}

void BLEPeripheralManager::SetServicesSetCallback(std::function<void(const char*)> cb) {
    servicesSetCallback = cb;
}

// Private emit methods
void BLEPeripheralManager::EmitStateChange(const char* state) {
    if (stateChangeCallback) {
        stateChangeCallback(state);
    }
}

void BLEPeripheralManager::EmitAdvertisingStart(const char* error) {
    if (advertisingStartCallback) {
        advertisingStartCallback(error);
    }
}

void BLEPeripheralManager::EmitAdvertisingStop() {
    if (advertisingStopCallback) {
        advertisingStopCallback();
    }
}

void BLEPeripheralManager::EmitServicesSet(const char* error) {
    if (servicesSetCallback) {
        servicesSetCallback(error);
    }
}

// BlenoWinRT implementation
BlenoWinRT::BlenoWinRT(const Napi::CallbackInfo& info) 
    : Napi::ObjectWrap<BlenoWinRT>(info) {
    // peripheralManager = std::make_unique<BLEPeripheralManager>();
}

Napi::Value BlenoWinRT::Init(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    Napi::Function emit = info.This().As<Napi::Object>().Get("emit").As<Napi::Function>();
    peripheralManager = new BLEPeripheralManager(info.This(), emit);

    if (!peripheralManager) {
        Napi::Error::New(env, "BLEPeripheralManager not initialized").ThrowAsJavaScriptException();
        return env.Undefined();
    }
    
    // Set up callbacks using the emit function
    // TODO: Implement proper callback bindings

    peripheralManager->Start();
    return env.Undefined();
}

Napi::Value BlenoWinRT::StartAdvertising(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsArray()) {
        Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
        return env.Undefined();
    }

    std::string name = info[0].As<Napi::String>().Utf8Value();
    std::vector<GUID> serviceUuids;
    // TODO: Convert JS array to vector<GUID>

    peripheralManager->StartAdvertising(name, serviceUuids);
    return env.Undefined();
}

Napi::Value BlenoWinRT::StopAdvertising(const Napi::CallbackInfo& info) {
    peripheralManager->StopAdvertising();
    return info.Env().Undefined();
}

Napi::Value BlenoWinRT::SetServices(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    
    if (info.Length() < 1 || !info[0].IsArray()) {
        Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
        return env.Undefined();
    }

    std::vector<GattLocalService> services;
    // TODO: Convert JS services array to vector<GattLocalService>
    
    peripheralManager->SetServices(services);
    return env.Undefined();
}

Napi::Value BlenoWinRT::Disconnect(const Napi::CallbackInfo& info) {
    peripheralManager->Disconnect();
    return info.Env().Undefined();
}

Napi::Value BlenoWinRT::UpdateRssi(const Napi::CallbackInfo& info) {
    peripheralManager->UpdateRssi();
    return info.Env().Undefined();
}

Napi::Value BlenoWinRT::Stop(const Napi::CallbackInfo& info) {
    peripheralManager->Stop();
    return info.Env().Undefined();
}

Napi::Object BlenoWinRT::Init(Napi::Env env, Napi::Object exports) {
    try {
        winrt::init_apartment();
    } catch (winrt::hresult_error hresult) {
        // electron already initialized the COM library
        if (hresult.code() != RPC_E_CHANGED_MODE) {
            wprintf(L"Failed initializing apartment: %d %s", hresult.code().value, hresult.message().c_str());
            Napi::TypeError::New(env, "Failed initializing apartment").ThrowAsJavaScriptException();
            return exports;
        }
    }

    Napi::Function func = DefineClass(env, "BlenoWinRT", {
        InstanceMethod("init", &BlenoWinRT::Init),
        InstanceMethod("startAdvertising", &BlenoWinRT::StartAdvertising),
        InstanceMethod("stopAdvertising", &BlenoWinRT::StopAdvertising),
        InstanceMethod("setServices", &BlenoWinRT::SetServices),
        InstanceMethod("disconnect", &BlenoWinRT::Disconnect),
        InstanceMethod("updateRssi", &BlenoWinRT::UpdateRssi),
        InstanceMethod("stop", &BlenoWinRT::Stop),
    });

    Napi::FunctionReference* constructor = new Napi::FunctionReference();
    *constructor = Napi::Persistent(func);
    env.SetInstanceData(constructor);

    exports.Set("BlenoWinRT", func);
    return exports;
}

NODE_API_NAMED_ADDON(addon, BlenoWinRT);
