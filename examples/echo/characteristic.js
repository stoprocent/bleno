const { Characteristic } = require('../../');

class EchoCharacteristic extends Characteristic {
  constructor () {
    super({
      uuid: 'ec0e',
      properties: ['read', 'write', 'notify'],
      value: null
    });

    this._value = Buffer.alloc(0);
    this._updateValueCallbacks = new Map();
  }

  onReadRequest (connection, offset, callback) {
    console.log('EchoCharacteristic - onReadRequest: handle = ' + connection + ' value = ' + this._value.toString('hex'));
    callback(this.RESULT_SUCCESS, this._value);
  }

  onWriteRequest (connection, data, offset, withoutResponse, callback) {
    this._value = data;
    console.log('EchoCharacteristic - onWriteRequest: handle = ' + connection + ' value = ' + this._value.toString('hex'));

    for (const updateValueCallback of this._updateValueCallbacks.values()) {
      console.log('EchoCharacteristic - onWriteRequest: notifying');
      updateValueCallback(this._value);
    }

    callback(this.RESULT_SUCCESS);
  }

  onSubscribe (connection, maxValueSize, updateValueCallback) {
    console.log('EchoCharacteristic - onSubscribe: handle = ' + connection);
    this._updateValueCallbacks.set(connection, updateValueCallback);
  }

  onUnsubscribe (connection) {
    console.log('EchoCharacteristic - onUnsubscribe: handle = ' + connection);
    this._updateValueCallbacks.delete(connection);
  }
}

module.exports = EchoCharacteristic;
