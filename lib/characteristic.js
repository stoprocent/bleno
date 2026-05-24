const BlenoEventEmitter = require('./bleno-event-emitter');
const UuidUtil = require('./uuid-util');

class Characteristic extends BlenoEventEmitter {
  constructor (options) {
    super();

    this.uuid = UuidUtil.removeDashes(options.uuid);
    this.properties = options.properties || [];
    this.secure = options.secure || [];
    this.value = options.value || null;
    this.descriptors = options.descriptors || [];

    if (this.value && (this.properties.length !== 1 || this.properties[0] !== 'read')) {
      throw new Error('Characteristics with value can be read only!');
    }

    if (options.onReadRequest) {
      this.onReadRequest = options.onReadRequest;
    }

    if (options.onWriteRequest) {
      this.onWriteRequest = options.onWriteRequest;
    }

    if (options.onSubscribe) {
      this.onSubscribe = options.onSubscribe;
    }

    if (options.onUnsubscribe) {
      this.onUnsubscribe = options.onUnsubscribe;
    }

    if (options.onNotify) {
      this.onNotify = options.onNotify;
    }

    if (options.onIndicate) {
      this.onIndicate = options.onIndicate;
    }

    this.on('readRequest', this.onReadRequest.bind(this));
    this.on('writeRequest', this.onWriteRequest.bind(this));
    this.on('subscribe', this.onSubscribe.bind(this));
    this.on('unsubscribe', this.onUnsubscribe.bind(this));
    this.on('notify', this.onNotify.bind(this));
    this.on('indicate', this.onIndicate.bind(this));

    this._maxValueSizes = new Map();
    this._updateValueCallbacks = new Map();
  }

  toString () {
    return JSON.stringify({
      uuid: this.uuid,
      properties: this.properties,
      secure: this.secure,
      value: this.value,
      descriptors: this.descriptors
    });
  }

  getMaxValueSize (connection) {
    return this._maxValueSizes.get(connection);
  }

  getUpdateValueCallback (connection) {
    return this._updateValueCallbacks.get(connection);
  }

  onReadRequest (connection, offset, callback) {
    callback(this.RESULT_UNLIKELY_ERROR, null);
  }

  onWriteRequest (connection, data, offset, withoutResponse, callback) {
    callback(this.RESULT_UNLIKELY_ERROR);
  }

  onSubscribe (connection, maxValueSize, updateValueCallback) {
    this._maxValueSizes.set(connection, maxValueSize);
    this._updateValueCallbacks.set(connection, updateValueCallback);
  }

  onUnsubscribe (connection) {
    this._maxValueSizes.delete(connection);
    this._updateValueCallbacks.delete(connection);
  }

  onNotify (connection) {
  }

  onIndicate (connection) {
  }

  notify (data, connection = null) {
    if (connection && this._updateValueCallbacks.has(connection)) {
      this._updateValueCallbacks.get(connection)(data);
    } else if (!connection) {
      for (const callback of this._updateValueCallbacks.values()) {
        callback(data);
      }
    }
  }
}

Characteristic.RESULT_SUCCESS = Characteristic.prototype.RESULT_SUCCESS = 0x00;
Characteristic.RESULT_INVALID_OFFSET = Characteristic.prototype.RESULT_INVALID_OFFSET = 0x07;
Characteristic.RESULT_ATTR_NOT_LONG = Characteristic.prototype.RESULT_ATTR_NOT_LONG = 0x0b;
Characteristic.RESULT_INVALID_ATTRIBUTE_LENGTH = Characteristic.prototype.RESULT_INVALID_ATTRIBUTE_LENGTH = 0x0d;
Characteristic.RESULT_UNLIKELY_ERROR = Characteristic.prototype.RESULT_UNLIKELY_ERROR = 0x0e;

module.exports = Characteristic;
