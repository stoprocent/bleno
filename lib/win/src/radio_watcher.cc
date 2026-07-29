#include "radio_watcher.h"

#include <windows.h>
#include <winrt/Windows.Devices.Bluetooth.h>

using winrt::Windows::Devices::Bluetooth::BluetoothAdapter;
using winrt::Windows::Devices::Radios::RadioState;

const char* AdapterStateToString(AdapterState state) {
    switch (state) {
    case AdapterState::On:
        return "poweredOn";
    case AdapterState::Off:
    case AdapterState::Disabled:
        return "poweredOff";
    case AdapterState::Unsupported:
        return "unsupported";
    case AdapterState::Unauthorized:
        return "unauthorized";
    default:
        return "unknown";
    }
}

RadioWatcher::RadioWatcher()
    : mWatcher(
        winrt::Windows::Devices::Enumeration::DeviceInformation::CreateWatcher(
            RadioInterfaceSelector)) {
    mAdded = mWatcher.Added(
        winrt::auto_revoke,
        { this, &RadioWatcher::OnAdded });
    mUpdated = mWatcher.Updated(
        winrt::auto_revoke,
        { this, &RadioWatcher::OnUpdated });
    mRemoved = mWatcher.Removed(
        winrt::auto_revoke,
        { this, &RadioWatcher::OnRemoved });
    mCompleted = mWatcher.EnumerationCompleted(
        winrt::auto_revoke,
        { this, &RadioWatcher::OnCompleted });
}

RadioWatcher::~RadioWatcher() {
    Stop();
}

void RadioWatcher::Start(std::function<void(AdapterState)> callback) {
    mCallback = std::move(callback);
    mStopped = false;
    mEnumerating = true;
    mWatcher.Start();
}

void RadioWatcher::Stop() noexcept {
    if (mStopped.exchange(true)) {
        return;
    }

    try {
        mRadioStateChanged.revoke();
        const auto status = mWatcher.Status();
        if (status == winrt::Windows::Devices::Enumeration::DeviceWatcherStatus::Started ||
            status == winrt::Windows::Devices::Enumeration::DeviceWatcherStatus::EnumerationCompleted) {
            mWatcher.Stop();
        }
    } catch (...) {
    }
    mCallback = nullptr;
    mRadio = nullptr;
}

winrt::fire_and_forget RadioWatcher::Refresh() {
    try {
        auto adapter = co_await BluetoothAdapter::GetDefaultAsync();
        if (mStopped) {
            co_return;
        }

        if (!adapter ||
            !adapter.IsLowEnergySupported() ||
            !adapter.IsPeripheralRoleSupported()) {
            mRadioStateChanged.revoke();
            mRadio = nullptr;
            Publish(AdapterState::Unsupported);
            co_return;
        }

        auto radio = co_await adapter.GetRadioAsync();
        if (mStopped) {
            co_return;
        }

        if (!radio) {
            mRadioStateChanged.revoke();
            mRadio = nullptr;
            Publish(AdapterState::Unsupported);
            co_return;
        }

        if (!mRadio || radio != mRadio) {
            mRadioStateChanged.revoke();
            mRadio = radio;
            mRadioStateChanged = mRadio.StateChanged(
                winrt::auto_revoke,
                [this](const auto& sender, const auto&) {
                    if (!mStopped) {
                        Publish(static_cast<AdapterState>(sender.State()));
                    }
                });
        }

        Publish(static_cast<AdapterState>(mRadio.State()));
    } catch (const winrt::hresult_error& error) {
        if (!mStopped) {
            mRadioStateChanged.revoke();
            mRadio = nullptr;
            Publish(
                error.code() == E_ACCESSDENIED
                    ? AdapterState::Unauthorized
                    : AdapterState::Unsupported);
        }
    } catch (...) {
        if (!mStopped) {
            mRadioStateChanged.revoke();
            mRadio = nullptr;
            Publish(AdapterState::Unsupported);
        }
    }
}

void RadioWatcher::OnAdded(
    const winrt::Windows::Devices::Enumeration::DeviceWatcher&,
    const winrt::Windows::Devices::Enumeration::DeviceInformation&) {
    if (!mEnumerating && !mStopped) {
        Refresh();
    }
}

void RadioWatcher::OnUpdated(
    const winrt::Windows::Devices::Enumeration::DeviceWatcher&,
    const winrt::Windows::Devices::Enumeration::DeviceInformationUpdate&) {
    if (!mStopped) {
        Refresh();
    }
}

void RadioWatcher::OnRemoved(
    const winrt::Windows::Devices::Enumeration::DeviceWatcher&,
    const winrt::Windows::Devices::Enumeration::DeviceInformationUpdate&) {
    if (!mStopped) {
        Refresh();
    }
}

void RadioWatcher::OnCompleted(
    const winrt::Windows::Devices::Enumeration::DeviceWatcher&,
    const winrt::Windows::Foundation::IInspectable&) {
    mEnumerating = false;
    if (!mStopped) {
        Refresh();
    }
}

void RadioWatcher::Publish(AdapterState state) {
    if (mStopped || !mCallback || state == mLastState) {
        return;
    }
    mLastState = state;
    mCallback(state);
}
