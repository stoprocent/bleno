#pragma once

#include <atomic>
#include <functional>

#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Radios.h>
#include <winrt/Windows.Foundation.h>

enum class AdapterState : int32_t {
    Unauthorized = -3,
    Initial = -2,
    Unsupported = -1,
    Unknown = static_cast<int32_t>(
        winrt::Windows::Devices::Radios::RadioState::Unknown),
    On = static_cast<int32_t>(
        winrt::Windows::Devices::Radios::RadioState::On),
    Off = static_cast<int32_t>(
        winrt::Windows::Devices::Radios::RadioState::Off),
    Disabled = static_cast<int32_t>(
        winrt::Windows::Devices::Radios::RadioState::Disabled),
};

const char* AdapterStateToString(AdapterState state);

class RadioWatcher {
public:
    RadioWatcher();
    ~RadioWatcher();

    void Start(std::function<void(AdapterState)> callback);
    void Stop() noexcept;

private:
    static constexpr const wchar_t* RadioInterfaceSelector =
        L"System.Devices.InterfaceClassGuid:=\"{A8804298-2D5F-42E3-9531-9C8C39EB29CE}\"";

    winrt::fire_and_forget Refresh();
    void OnAdded(
        const winrt::Windows::Devices::Enumeration::DeviceWatcher&,
        const winrt::Windows::Devices::Enumeration::DeviceInformation&);
    void OnUpdated(
        const winrt::Windows::Devices::Enumeration::DeviceWatcher&,
        const winrt::Windows::Devices::Enumeration::DeviceInformationUpdate&);
    void OnRemoved(
        const winrt::Windows::Devices::Enumeration::DeviceWatcher&,
        const winrt::Windows::Devices::Enumeration::DeviceInformationUpdate&);
    void OnCompleted(
        const winrt::Windows::Devices::Enumeration::DeviceWatcher&,
        const winrt::Windows::Foundation::IInspectable&);
    void Publish(AdapterState state);

    std::atomic<bool> mStopped{ true };
    std::atomic<bool> mEnumerating{ false };
    AdapterState mLastState{ AdapterState::Initial };
    std::function<void(AdapterState)> mCallback;

    winrt::Windows::Devices::Enumeration::DeviceWatcher mWatcher{ nullptr };
    winrt::Windows::Devices::Radios::Radio mRadio{ nullptr };

    winrt::Windows::Devices::Enumeration::DeviceWatcher::Added_revoker mAdded;
    winrt::Windows::Devices::Enumeration::DeviceWatcher::Updated_revoker mUpdated;
    winrt::Windows::Devices::Enumeration::DeviceWatcher::Removed_revoker mRemoved;
    winrt::Windows::Devices::Enumeration::DeviceWatcher::EnumerationCompleted_revoker mCompleted;
    winrt::Windows::Devices::Radios::Radio::StateChanged_revoker mRadioStateChanged;
};
