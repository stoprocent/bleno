const debug = require('debug')('bleno');
const UuidUtil = require('./uuid-util');

const { EventEmitter } = require('events');

const PrimaryService = require('./primary-service');
const Characteristic = require('./characteristic');
const Descriptor = require('./descriptor');

class Bleno extends EventEmitter {
  constructor (bindings) {
    super();
    this.initialized = false;
    this.platform = 'unknown';
    this.state = 'unknown';
    this.address = 'unknown';
    this.rssi = 0;
    this.mtu = 20;

    this._bindings = bindings;

    this._bindings.on('stateChange', this.onStateChange.bind(this));
    this._bindings.on('platform', this.onPlatform.bind(this));
    this._bindings.on('addressChange', this.onAddressChange.bind(this));
    this._bindings.on('advertisingStart', this.onAdvertisingStart.bind(this));
    this._bindings.on('advertisingStop', this.onAdvertisingStop.bind(this));
    this._bindings.on('servicesSet', this.onServicesSet.bind(this));
    this._bindings.on('accept', this.onAccept.bind(this));
    this._bindings.on('mtuChange', this.onMtuChange.bind(this));
    this._bindings.on('disconnect', this.onDisconnect.bind(this));
    this._bindings.on('rssiUpdate', this.onRssiUpdate.bind(this));

    this.on('newListener', (event) => {
      if (event === 'stateChange' && !this.initialized) {
        this._bindings.init();

        this.initialized = true;
      }
    });
  }

  onPlatform (platform) {
    debug('platform ' + platform);

    this.platform = platform;
  }

  onStateChange (state) {
    debug('stateChange ' + state);

    this.state = state;

    this.emit('stateChange', state);
  }

  onAddressChange (address) {
    debug('addressChange ' + address);

    this.address = address;
  }

  onAccept (clientAddress, handle) {
    debug('accept ' + clientAddress + ' ' + handle);
    this.emit('accept', clientAddress, handle);
  }

  onMtuChange (mtu) {
    debug('mtu ' + mtu);
    this.mtu = mtu;
    this.emit('mtuChange', mtu);
  }

  onDisconnect (clientAddress, handle) {
    debug('disconnect ' + clientAddress + ' ' + handle);
    this.emit('disconnect', clientAddress, handle);
  }

  async waitForPoweredOnAsync (timeout = 10000) {
    if (this.state === 'poweredOn') {
      return;
    }

    return new Promise((resolve, reject) => {
      const timeoutId = setTimeout(() => {
        this.removeListener('stateChange', stateChangeHandler);
        reject(new Error(`Timeout waiting for poweredOn state. Current state: ${this.state}`));
      }, timeout);

      const stateChangeHandler = (state) => {
        if (state === 'poweredOn') {
          clearTimeout(timeoutId);
          this.removeListener('stateChange', stateChangeHandler);
          resolve();
        }
      };

      this.on('stateChange', stateChangeHandler);
    });
  }

  setAddress (address) {
    if (this._bindings.setAddress) {
      this._bindings.setAddress(address);
    } else {
      this.emit('warning', 'current binding does not implement setAddress method.');
    }
  }

  setAddressAsync (address) {
    if (this._bindings.setAddress === undefined) {
      return Promise.resolve();
    }
    return new Promise((resolve, reject) => {
      const timeout = setTimeout(() => {  
        reject(new Error('Timeout waiting for address change'));
      }, 2000);
      this._bindings.once('addressChange', (address) => {
        if (address === address) {
          clearTimeout(timeout);
          resolve();
        }
        else {
          reject(new Error('Address change failed'));
        }
      });
      this.setAddress(address);
    });
  }

  startAdvertising (name, serviceUuids, callback) {
    if (this.state !== 'poweredOn') {
      const error = new Error('Could not start advertising, state is ' + this.state + ' (not poweredOn)');

      if (typeof callback === 'function') {
        callback(error);
      } else {
        throw error;
      }
    } else {
      if (typeof callback === 'function') {
        this.once('advertisingStart', callback);
      }

      const undashedServiceUuids = [];

      if (serviceUuids && serviceUuids.length) {
        for (let i = 0; i < serviceUuids.length; i++) {
          undashedServiceUuids[i] = UuidUtil.removeDashes(serviceUuids[i]);
        }
      }

      this._bindings.startAdvertising(name, undashedServiceUuids);
    }
  }

  startAdvertisingAsync (name, serviceUuids) {
    return new Promise((resolve, reject) => {
      this.startAdvertising(name, serviceUuids, (error) => {
        if (error) reject(error);
        else resolve();
      });
    });
  }

  startAdvertisingIBeacon (uuid, major, minor, measuredPower, callback) {
    if (this.state !== 'poweredOn') {
      const error = new Error('Could not start advertising, state is ' + this.state + ' (not poweredOn)');

      if (typeof callback === 'function') {
        callback(error);
      } else {
        throw error;
      }
    } else {
      const undashedUuid = UuidUtil.removeDashes(uuid);
      const uuidData = Buffer.from(undashedUuid, 'hex');
      const uuidDataLength = uuidData.length;
      const iBeaconData = Buffer.alloc(uuidData.length + 5);

      for (let i = 0; i < uuidDataLength; i++) {
        iBeaconData[i] = uuidData[i];
      }

      iBeaconData.writeUInt16BE(major, uuidDataLength);
      iBeaconData.writeUInt16BE(minor, uuidDataLength + 2);
      iBeaconData.writeInt8(measuredPower, uuidDataLength + 4);

      if (typeof callback === 'function') {
        this.once('advertisingStart', callback);
      }

      debug('iBeacon data = ' + iBeaconData.toString('hex'));

      this._bindings.startAdvertisingIBeacon(iBeaconData);
    }
  }

  startAdvertisingIBeaconAsync (uuid, major, minor, measuredPower) {
    return new Promise((resolve, reject) => {
      this.startAdvertisingIBeacon(uuid, major, minor, measuredPower, (error) => {
        if (error) reject(error);
        else resolve();
      });
    });
  }

  onAdvertisingStart (error) {
    debug('advertisingStart: ' + error);
    this.emit('advertisingStart', error);
  }

  startAdvertisingWithEIRData (advertisementData, scanData, callback) {
    if (typeof scanData === 'function') {
      callback = scanData;
      scanData = null;
    }

    if (this.state !== 'poweredOn') {
      const error = new Error('Could not advertising scanning, state is ' + this.state + ' (not poweredOn)');

      if (typeof callback === 'function') {
        callback(error);
      } else {
        throw error;
      }
    } else {
      if (typeof callback === 'function') {
        this.once('advertisingStart', callback);
      }

      this._bindings.startAdvertisingWithEIRData(advertisementData, scanData);
    }
  }

  startAdvertisingWithEIRDataAsync (advertisementData, scanData) {
    return new Promise((resolve, reject) => {
      this.startAdvertisingWithEIRData(advertisementData, scanData, (error) => {
        if (error) reject(error);
        else resolve();
      });
    });
  }

  stopAdvertising (callback) {
    if (typeof callback === 'function') {
      this.once('advertisingStop', callback);
    }
    this._bindings.stopAdvertising();
  }

  stopAdvertisingAsync () {
    return new Promise((resolve, reject) => {
      this.stopAdvertising((error) => {
        if (error) reject(error);
        else resolve();
      });
    });
  }

  onAdvertisingStop () {
    debug('advertisingStop');
    this.emit('advertisingStop');
  }

  setServices (services, callback) {
    if (typeof callback === 'function') {
      this.once('servicesSet', callback);
    }
    this._bindings.setServices(services);
  }

  setServicesAsync (services) {
    return new Promise((resolve, reject) => {
      this.setServices(services, (error) => {
        if (error) reject(error);
        else resolve();
      });
    });
  }

  onServicesSet (error) {
    debug('servicesSet');

    if (error) {
      this.emit('servicesSetError', error);
    } else {
      this.emit('servicesSet', error);
    }
  }

  disconnect (handle = null) {
    debug('disconnect ' + (handle ? handle : 'all'));
    this._bindings.disconnect(handle);
  }

  stop () {
    debug('stop');
    this._bindings.stop();
  }

  updateRssi (callback) {
    if (typeof callback === 'function') {
      this.once('rssiUpdate', (rssi) => callback(null, rssi));
    }

    this._bindings.updateRssi();
  }

  updateRssiAsync () {
    return new Promise((resolve, reject) => {
      this.updateRssi((error) => {
        if (error) reject(error);
        else resolve();
      });
    });
  }

  onRssiUpdate (rssi) {
    this.emit('rssiUpdate', rssi);
  }
}

Bleno.prototype.PrimaryService = PrimaryService;
Bleno.prototype.Characteristic = Characteristic;
Bleno.prototype.Descriptor = Descriptor;

module.exports = Bleno;
