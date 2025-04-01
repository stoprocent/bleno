# bleno

[![GitHub forks](
https://img.shields.io/github/forks/stoprocent/bleno.svg?style=social&label=Fork&maxAge=2592000
)](
https://GitHub.com/stoprocent/bleno/network/
)
[![license](
https://img.shields.io/badge/license-MIT-0.svg
)](MIT)
[![NPM](
https://img.shields.io/npm/v/@stoprocent/bleno.svg
)](
https://www.npmjs.com/package/@stoprocent/bleno
)

A Node.js module for implementing BLE (Bluetooth Low Energy) peripherals.

Need a BLE central module? See [@stoprocent/noble](https://github.com/stoprocent/noble).

## About This Fork

This fork of `bleno` was created to introduce several key improvements and new features:

1. **HCI UART Support**: This version enables HCI UART communication through the `@stoprocent/node-bluetooth-hci-socket` dependency, allowing more flexible use of Bluetooth devices across platforms.

2. **Multi-Connection Support**: Added support for multiple concurrent BLE central connections, allowing the peripheral to handle several connected devices simultaneously. This enables scenarios where multiple clients can connect and interact with the peripheral at the same time.

3. **macOS Native Bindings Fix**: I have fixed the native bindings for macOS, ensuring better compatibility and performance on Apple devices.

4. **Modern Async API**: Added Promise-based async methods for all major operations, making it easier to use with modern JavaScript/TypeScript applications.

5. **TypeScript Support**: Full TypeScript definitions included for better development experience and type safety.

6. **New Features**: A `setAddress` function has been added, allowing users to set the MAC address of the peripheral device. Additionally, I plan to add raw L2CAP channel support, enhancing low-level Bluetooth communication capabilities.

If you appreciate these enhancements and the continued development of this project, please consider supporting my work.

[![Buy me a coffee](https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png)](https://www.buymeacoffee.com/stoprocent)


## Install

```sh
npm install @stoprocent/bleno --save
```

## Usage

#### TypeScript
``` typescript
import bleno from "@stoprocent/bleno";
```

#### JavaScript
```javascript
const bleno = require('@stoprocent/bleno');
```

See [examples folder](https://github.com/stoprocent/bleno/blob/master/examples) for code examples.


## Prerequisites

### Import

#### Binding will be picked automatically

``` typescript
import bleno from "@stoprocent/bleno";
```

#### Binding picked manually

``` typescript
import { withBindings } from "@stoprocent/bleno";

const bleno = withBindings('default');
const bleno = withBindings('mac');
const bleno = withBindings('hci', { hciDriver: '...', bindParams: ... });
```

> For available `hciDriver: DriverType` and `bindParams: BindParams` please refer to [@stoprocent/bluetooth-hci-socket](https://github.com/stoprocent/node-bluetooth-hci-socket/blob/main/index.d.ts) interface.

#### UART (Any OS)

Please refer to [https://github.com/stoprocent/node-bluetooth-hci-socket#uartserial-any-os](https://github.com/stoprocent/node-bluetooth-hci-socket#uartserial-any-os)

##### Example 1 (UART port specified as environmental variable)

```bash
$ export BLUETOOTH_HCI_SOCKET_UART_PORT=/dev/tty...
$ export BLUETOOTH_HCI_SOCKET_UART_BAUDRATE=1000000
```

__NOTE:__ `BLUETOOTH_HCI_SOCKET_UART_BAUDRATE` defaults to `1000000` so only needed if different.

```javascript
const bleno = require('@stoprocent/bleno');
```

##### Example 2 (UART port specified in `bindParams`)

```typescript
import { withBindings } from "@stoprocent/bleno";

const bleno = withBindings('hci', { 
    hciDriver: 'uart', 
    bindParams: { 
        uart: { 
            port: '/dev/cu.usbserial-1420' 
        } 
    } 
});

await bleno.waitForPoweredOnAsync(1000); // Timeout of 1s
await bleno.startAdvertisingAsync('hello', ['d00d', 'b00b']);
```

### Multi-Connection Support

This fork of bleno supports multiple concurrent BLE central connections, allowing several clients to connect to your peripheral simultaneously. 

The library automatically restarts advertising after accepting a connection, enabling seamless multi-client connectivity by default. If you want to prevent additional connections, you can explicitly stop advertising in the accept callback:

```typescript
// Default behavior - allows multiple connections
bleno.on('accept', async (address, handle) => {
  console.log(`New connection accepted: ${address}, handle: ${handle}`);
  // Advertising automatically continues - no action needed
});

// Optional: Stop additional connections
bleno.on('accept', async (address, handle) => {
  console.log(`New connection accepted: ${address}, handle: ${handle}`);
  // Explicitly stop advertising if you want to prevent additional connections
  await bleno.stopAdvertisingAsync();
});
```

If you stop advertising, you'll need to manually restart it after the last client disconnects to keep the device connectable.

```typescript
bleno.on('disconnect', async (address, handle) => {
  console.log(`Client disconnected: ${address}, handle: ${handle}`);
  // Only need to restart advertising if you explicitly stopped it
  // await bleno.startAdvertisingAsync('my-device', ['service-uuid']);
});
```

### OS X

 * install the XCode command line tools via `xcode-select --install`
 * 10.9 or later

### Linux

 * Kernel version 3.6 or above
 * ```libbluetooth-dev```
 * ```bluetoothd``` disabled, if BlueZ 5.14 or later is installed. Use ```sudo hciconfig hci0 up``` to power Bluetooth adapter up after stopping or disabling ```bluetoothd```.
    * ```System V```:
      * ```sudo service bluetooth stop``` (once)
      * ```sudo update-rc.d bluetooth remove``` (persist on reboot)
    * ```systemd```
      * ```sudo systemctl stop bluetooth``` (once)
      * ```sudo systemctl disable bluetooth``` (persist on reboot)

If you're using [noble](https://github.com/sandeepmistry/noble) *and* bleno at the same time, connected BLE devices may not be able to retrieve a list of services from the BLE adaptor. Check out noble's [documentation on bleno compatibility](https://github.com/sandeepmistry/noble#bleno-compatibility)

#### Ubuntu/Debian/Raspbian

```sh
sudo apt-get install bluetooth bluez libbluetooth-dev libudev-dev libusb-1.0-0-dev
```

Make sure ```node``` is on your path, if it's not, some options:
 * symlink ```nodejs``` to ```node```: ```sudo ln -s /usr/bin/nodejs /usr/bin/node```
 * [install Node.js using the NodeSource package](https://nodejs.org/en/download/package-manager/#debian-and-ubuntu-based-linux-distributions)

#### Fedora / Other-RPM based

```sh
sudo yum install bluez bluez-libs bluez-libs-devel
```

#### Intel Edison

See [Configure Intel Edison for Bluetooth LE (Smart) Development](http://rexstjohn.com/configure-intel-edison-for-bluetooth-le-smart-development/)

### FreeBSD

Make sure you have GNU Make:

```sh
sudo pkg install gmake
```

Disable automatic loading of the default Bluetooth stack by putting [no-ubt.conf](https://gist.github.com/myfreeweb/44f4f3e791a057bc4f3619a166a03b87) into ```/usr/local/etc/devd/no-ubt.conf``` and restarting devd (```sudo service devd restart```).

Unload ```ng_ubt``` kernel module if already loaded:

```sh
sudo kldunload ng_ubt
```

Make sure you have read and write permissions on the ```/dev/usb/*``` device that corresponds to your Bluetooth adapter.

### Windows

 * [node-gyp requirements for Windows](https://github.com/TooTallNate/node-gyp#installation)
   * Python 2.7
   * Visual Studio ([Express](https://www.visualstudio.com/en-us/products/visual-studio-express-vs.aspx))
 * [node-bluetooth-hci-socket prerequisites](https://github.com/sandeepmistry/node-bluetooth-hci-socket#windows)
   * Compatible Bluetooth 4.0 USB adapter
   * [WinUSB](https://msdn.microsoft.com/en-ca/library/windows/hardware/ff540196(v=vs.85).aspx) driver setup for Bluetooth 4.0 USB adapter, using [Zadig tool](http://zadig.akeo.ie/)

## API Reference

### Actions

#### Wait for powered on state

```typescript
// Promise-based
await bleno.waitForPoweredOnAsync(timeout: number); // timeout in milliseconds
```

#### Set address

```javascript
// Callback-based
bleno.setAddress('00:11:22:33:44:55'); // set adapter's mac address

// Promise-based
await bleno.setAddressAsync('00:11:22:33:44:55');
```
__NOTE:__ Currently this feature is only supported on HCI as it's using vendor specific commands. Source of the commands is based on the [BlueZ bdaddr.c](https://github.com/pauloborges/bluez/blob/master/tools/bdaddr.c).
__NOTE:__ `bleno.state` must be `poweredOn` before address can be set. `bleno.on('stateChange', callback(state));` can be used to listen for state change events.

#### Advertising

##### Start advertising

NOTE: ```bleno.state``` must be ```poweredOn``` before advertising is started. ```bleno.on('stateChange', callback(state));``` can be used register for state change events.

```javascript
// Callback-based
var name = 'name';
var serviceUuids = ['fffffffffffffffffffffffffffffff0']
bleno.startAdvertising(name, serviceUuids[, callback(error)]);

// Promise-based
await bleno.startAdvertisingAsync(name, serviceUuids);
```

 __Note:__: there are limits on the name and service UUID's

  * name
    * maximum 26 bytes
  * service UUID's
    * 1 128-bit service UUID
    * 1 128-bit service UUID + 2 16-bit service UUID's
    * 7 16-bit service UUID


##### Start advertising iBeacon

```javascript
// Callback-based
var uuid = 'e2c56db5dffb48d2b060d0f5a71096e0';
var major = 0; // 0x0000 - 0xffff
var minor = 0; // 0x0000 - 0xffff
var measuredPower = -59; // -128 - 127

bleno.startAdvertisingIBeacon(uuid, major, minor, measuredPower[, callback(error)]);

// Promise-based
await bleno.startAdvertisingIBeaconAsync(uuid, major, minor, measuredPower);
```

 __Notes:__:
  * OS X:
    * in iBeacon mode your peripheral is non-connectable!

##### Start advertising with EIR data (__Linux only__)

```javascript
// Callback-based
var scanData = Buffer.alloc(...); // maximum 31 bytes
var advertisementData = Buffer.alloc(...); // maximum 31 bytes

bleno.startAdvertisingWithEIRData(advertisementData[, scanData, callback(error)]);

// Promise-based
await bleno.startAdvertisingWithEIRDataAsync(advertisementData[, scanData]);
```

  * For EIR format section [Bluetooth Core Specification](https://www.bluetooth.org/docman/handlers/downloaddoc.ashx?doc_id=229737) sections and 8 and 18 for more information the data format.

##### Stop advertising

```javascript
// Callback-based
bleno.stopAdvertising([callback]);

// Promise-based
await bleno.stopAdvertisingAsync();
```

#### Set services

Set the primary services available on the peripheral.

```javascript
// Callback-based
var services = [
   ... // see PrimaryService for data type
];

bleno.setServices(services[, callback(error)]);

// Promise-based
await bleno.setServicesAsync(services);
```

#### Disconnect client

```javascript
bleno.disconnect(); // Linux only
```

#### Update RSSI

```javascript
// Callback-based
bleno.updateRssi([callback(error, rssi)]); // not available in OS X 10.9

// Promise-based
const rssi = await bleno.updateRssiAsync();
```

### Primary Service

```javascript
var PrimaryService = bleno.PrimaryService;

var primaryService = new PrimaryService({
    uuid: 'fffffffffffffffffffffffffffffff0', // or 'fff0' for 16-bit
    characteristics: [
        // see Characteristic for data type
    ]
});
```

### Characteristic

```javascript
var Characteristic = bleno.Characteristic;

var characteristic = new Characteristic({
    uuid: 'fffffffffffffffffffffffffffffff1', // or 'fff1' for 16-bit
    properties: [ ... ], // can be a combination of 'read', 'write', 'writeWithoutResponse', 'notify', 'indicate'
    secure: [ ... ], // enable security for properties, can be a combination of 'read', 'write', 'writeWithoutResponse', 'notify', 'indicate'
    value: null, // optional static value, must be of type Buffer - for read only characteristics
    descriptors: [
        // see Descriptor for data type
    ],
    onReadRequest: null, // optional read request handler, function(handle, offset, callback) { ... }
    onWriteRequest: null, // optional write request handler, function(handle, data, offset, withoutResponse, callback) { ...}
    onSubscribe: null, // optional notify/indicate subscribe handler, function(handle, maxValueSize, updateValueCallback) { ...}
    onUnsubscribe: null, // optional notify/indicate unsubscribe handler, function(handle) { ...}
    onNotify: null, // optional notify sent handler, function(handle) { ...}
    onIndicate: null // optional indicate confirmation received handler, function(handle) { ...}
});
```

#### Multi-Connection Support

With multi-connection support, each handler now receives a `handle` parameter to identify which client connection is making the request:

```javascript
// Example of a characteristic with multi-connection support
var multiConnectionCharacteristic = new Characteristic({
    uuid: 'fff1',
    properties: ['read', 'write', 'notify'],
    onReadRequest: function(handle, offset, callback) {
        console.log('Read request from connection: ' + handle);
        // Process the read request for this specific connection
        callback(Characteristic.RESULT_SUCCESS, Buffer.from('Data for connection ' + handle));
    },
    onWriteRequest: function(handle, data, offset, withoutResponse, callback) {
        console.log('Write request from connection: ' + handle);
        // Process the write request for this specific connection
        console.log('Data received: ' + data.toString());
        callback(Characteristic.RESULT_SUCCESS);
    },
    onSubscribe: function(handle, maxValueSize, updateValueCallback) {
        console.log('Subscribe from connection: ' + handle);
        // Store the callback for this specific connection to use later
        // You might want to store these in a Map keyed by handle
        connectionCallbacks.set(handle, updateValueCallback);
    },
    onUnsubscribe: function(handle) {
        console.log('Unsubscribe from connection: ' + handle);
        // Remove the callback for this connection
        connectionCallbacks.delete(handle);
    }
});

// Later, to notify a specific connection:
const callback = connectionCallbacks.get(someConnectionHandle);
if (callback) {
    callback(Buffer.from('Notification for connection ' + someConnectionHandle));
}
```

#### Result codes

  * Characteristic.RESULT_SUCCESS
  * Characteristic.RESULT_INVALID_OFFSET
  * Characteristic.RESULT_INVALID_ATTRIBUTE_LENGTH
  * Characteristic.RESULT_UNLIKELY_ERROR
  * Characteristic.RESULT_ATTR_NOT_LONG

#### Read requests

Can specify read request handler via constructor options or by extending Characteristic and overriding onReadRequest.

Parameters to handler are
  * ```handle``` (connection handle)
  * ```offset``` (0x0000 - 0xffff)
  * ```callback```


```callback``` must be called with result and data (of type ```Buffer```) - can be async.

```javascript
var result = Characteristic.RESULT_SUCCESS;
var data = Buffer.alloc( ... );

callback(result, data);
```

#### Write requests

Can specify write request handler via constructor options or by extending Characteristic and overriding onWriteRequest.

Parameters to handler are
  * ```handle``` (connection handle)
  * ```data``` (Buffer)
  * ```offset``` (0x0000 - 0xffff)
  * ```withoutResponse``` (true | false)
  * ```callback```.

```callback``` must be called with result code - can be async.

```javascript
var result = Characteristic.RESULT_SUCCESS;

callback(result);
```

#### Notify subscribe

Can specify notify subscribe handler via constructor options or by extending Characteristic and overriding onSubscribe.

Parameters to handler are
  * ```handle``` (connection handle)
  * ```maxValueSize``` (maximum data size)
  * ```updateValueCallback``` (callback to call when value has changed)

#### Notify unsubscribe

Can specify notify unsubscribe handler via constructor options or by extending Characteristic and overriding onUnsubscribe.

Parameters to handler are
  * ```handle``` (connection handle)

#### Notify value changes

Call the ```updateValueCallback``` callback (see Notify subscribe), with an argument of type ```Buffer```

Can specify notify sent handler via constructor options or by extending Characteristic and overriding onNotify.

Parameters to handler are
  * ```handle``` (connection handle)

### Descriptor

```javascript
var Descriptor = bleno.Descriptor;

var descriptor = new Descriptor({
    uuid: '2901',
    value: 'value' // static value, must be of type Buffer or string if set
});
```

### Events

#### Adapter state change

```javascript
state = <"unknown" | "resetting" | "unsupported" | "unauthorized" | "poweredOff" | "poweredOn">

bleno.on('stateChange', callback(state));
```

#### Platform

```javascript
bleno.on('platform', callback(platform));
```

#### Address change

```javascript
bleno.on('addressChange', callback(address));
```

#### MTU change

```javascript
bleno.on('mtuChange', callback(mtu));
```

#### Advertisement started

```javascript
bleno.on('advertisingStart', callback(error));

bleno.on('advertisingStartError', callback(error));
```

#### Advertisement stopped

```javascript
bleno.on('advertisingStop', callback);
```

#### Services set

```javascript
bleno.on('servicesSet', callback(error));

bleno.on('servicesSetError', callback(error));
```

#### Accept

```javascript
bleno.on('accept', callback(address, handle)); // handle provided for multi-connection support
```

#### Disconnect

```javascript
bleno.on('disconnect', callback(address, handle)); // handle provided for multi-connection support
```

#### RSSI Update

```javascript
bleno.on('rssiUpdate', callback(rssi));
```

### Running on Linux

__Note:__ Make sure you've also checked the [Linux Prerequisites](#linux)

#### Running without root/sudo

Run the following command:

```sh
sudo setcap cap_net_raw+eip $(eval readlink -f `which node`)
```

This grants the ```node``` binary ```cap_net_raw``` privileges, so it can start/stop BLE advertising.

__Note:__ The above command requires ```setcap``` to be installed, it can be installed using the following:

 * apt: ```sudo apt-get install libcap2-bin```
 * yum: ```su -c \'yum install libcap2-bin\'```

#### Multiple Adapters

```hci0``` is used by default to override set the ```BLENO_HCI_DEVICE_ID``` environment variable to the interface number.

Example, specify ```hci1```:

```sh
sudo BLENO_HCI_DEVICE_ID=1 node <your file>.js
```

#### Set custom device name

By default bleno uses the hostname (```require('os').hostname()```) as the value for the device name (0x2a00) characterisic, to match the behaviour of OS X.

A custom device name can be specified by setting the ```BLENO_DEVICE_NAME``` environment variable:

```sh
sudo BLENO_DEVICE_NAME="custom device name" node <your file>.js
```

or

```js
process.env['BLENO_DEVICE_NAME'] = 'custom device name';
```

#### Set Advertising Interval

bleno uses a 100 ms advertising interval by default.

A custom advertising interval can be specified by setting the ```BLENO_ADVERTISING_INTERVAL``` environment variable with the desired value in milliseconds:

```sh
sudo BLENO_ADVERTISING_INTERVAL=500 node <your file>.js
```

Advertising intervals must be between 20 ms to 10 s (10,000 ms).

## Useful tools/links

 * Tools
   * LightBlue for [iOS](https://itunes.apple.com/us/app/lightblue/id557428110)/[OS X](https://itunes.apple.com/us/app/lightblue/id639944780)
   * [nRF Master Control Panel (BLE)](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp&hl=en) for Android
   * [hcitool](http://linux.die.net/man/1/hcitool) and ```gatttool``` by [BlueZ](http://www.bluez.org) for Linux