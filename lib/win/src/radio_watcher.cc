//
//  radio_watcher.cc
//  noble-winrt-native
//
//  Created by Georg Vienna on 07.09.18.
//

#pragma once

#include "radio_watcher.h"
#include "winrt_cpp.h"
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/windows.devices.bluetooth.h>
#include <ppltasks.h>
#include <future>

#include <string>
#include <sstream>
#include <iomanip>
#include <cstdint>

using winrt::Windows::Devices::Radios::RadioKind;
using winrt::Windows::Foundation::AsyncStatus;
using namespace winrt;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Bluetooth;
using namespace Windows::Devices::Bluetooth::GenericAttributeProfile;

template <typename O, typename M, class... Types> auto bind2(O* object, M method, Types&... args)
{
    return std::bind(method, object, std::placeholders::_1, std::placeholders::_2, args...);
}

#define RADIO_INTERFACE_CLASS_GUID \
    L"System.Devices.InterfaceClassGuid:=\"{A8804298-2D5F-42E3-9531-9C8C39EB29CE}\""

const char* adapterStateToString(AdapterState state)
{
    switch (state)
    {
    case AdapterState::Unsupported:
        return "unsupported";
    case AdapterState::On:
        return "poweredOn";
        break;
    case AdapterState::Off:
        return "poweredOff";
        break;
    case AdapterState::Disabled:
        return "poweredOff";
        break;
    default:
        return "unknown";
    }
}

RadioWatcher::RadioWatcher()
    : mRadio(nullptr), watcher(DeviceInformation::CreateWatcher(RADIO_INTERFACE_CLASS_GUID))
{
    mAddedRevoker = watcher.Added(winrt::auto_revoke, bind2(this, &RadioWatcher::OnAdded));
    mUpdatedRevoker = watcher.Updated(winrt::auto_revoke, bind2(this, &RadioWatcher::OnUpdated));
    mRemovedRevoker = watcher.Removed(winrt::auto_revoke, bind2(this, &RadioWatcher::OnRemoved));
    auto completed = bind2(this, &RadioWatcher::OnCompleted);
    mCompletedRevoker = watcher.EnumerationCompleted(winrt::auto_revoke, completed);
}

void RadioWatcher::Start(std::function<void(Radio& radio)> on)
{
    radioStateChanged = on;
    inEnumeration = true;
    watcher.Start();
}

winrt::fire_and_forget RadioWatcher::OnRadioChanged() {
    try {
        auto adapter = co_await BluetoothAdapter::GetDefaultAsync();
        
        if (adapter) {
            auto radio = co_await adapter.GetRadioAsync();

            // Log adapter capabilities
            std::string narrow = formatBluetoothAddress(adapter.BluetoothAddress());
            std::wstring wide(narrow.begin(), narrow.end());
            wprintf(L"MAC Address of the adapter: %ls\n", wide.c_str());
            printf("Are Classic Secure Connections Supported %d\n", adapter.AreClassicSecureConnectionsSupported());
            printf("Are Low Energy Secure Connections Supported %d\n", adapter.AreLowEnergySecureConnectionsSupported());
            printf("Is Extended Advertising Supported %d\n", adapter.IsExtendedAdvertisingSupported());
            printf("Is Low Energy Supported %d\n", adapter.IsLowEnergySupported());
            printf("Max Advertisement Data Length %d\n", adapter.MaxAdvertisementDataLength());
            printf("Is Supported Peripheral %d\n", adapter.IsPeripheralRoleSupported());
            printf("Is Supported Central %d\n", adapter.IsCentralRoleSupported());

            Radio bluetooth = nullptr;
            if (radio.State() == RadioState::On && adapter.IsPeripheralRoleSupported()) {
                bluetooth = radio;
            }

            if (!bluetooth || bluetooth != mRadio) {
                if (bluetooth) {
                    mRadioStateChangedRevoker.revoke();
                    mRadioStateChangedRevoker = bluetooth.StateChanged(
                        winrt::auto_revoke, 
                        [this](Radio radio, auto&&) { 
                            radioStateChanged(radio); 
                        });
                } else {
                    mRadioStateChangedRevoker.revoke();
                }
                
                radioStateChanged(bluetooth);
                mRadio = bluetooth;
            }
        } else {
            mRadio = nullptr;
            mRadioStateChangedRevoker.revoke();
            radioStateChanged(mRadio);
        }
    } catch (const winrt::hresult_error& ex) {
        mRadio = nullptr;
        mRadioStateChangedRevoker.revoke();
        radioStateChanged(mRadio);
    }
}

void RadioWatcher::OnAdded(DeviceWatcher watcher, DeviceInformation info)
{
    if (inEnumeration) { return; }
    OnRadioChanged();
}

void RadioWatcher::OnUpdated(DeviceWatcher watcher, DeviceInformationUpdate info)
{
    OnRadioChanged();
}

void RadioWatcher::OnRemoved(DeviceWatcher watcher, DeviceInformationUpdate info)
{
    OnRadioChanged();
}

void RadioWatcher::OnCompleted(DeviceWatcher watcher, IInspectable info)
{
    inEnumeration = false;
    OnRadioChanged();
}
