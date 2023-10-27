const bleno = require('../..');

const BlenoCharacteristic = bleno.Characteristic;

class EchoCharacteristic extends BlenoCharacteristic {
  constructor () {
    super({
      uuid: 'ec0e',
      properties: ['read', 'write', 'notify'],
      value: null
    });

    this._value = Buffer.alloc(0);
    this._updateValueCallback = null;
  }

  onReadRequest (offset, callback) {
    console.log('EchoCharacteristic - onReadRequest: value = ' + this._value.toString('hex'));
    callback(this.RESULT_SUCCESS, this._value);
  }

  onWriteRequest (data, offset, withoutResponse, callback) {
    this._value = data;
    console.log('EchoCharacteristic - onWriteRequest: value = ' + this._value.toString('hex'));

    if (this._updateValueCallback) {
      console.log('EchoCharacteristic - onWriteRequest: notifying');
      this._updateValueCallback(this._value);
    }

    callback(this.RESULT_SUCCESS);
  }

  onSubscribe (maxValueSize, updateValueCallback) {
    console.log('EchoCharacteristic - onSubscribe');
    this._updateValueCallback = updateValueCallback;
  }

  onUnsubscribe () {
    console.log('EchoCharacteristic - onUnsubscribe');
    this._updateValueCallback = null;
  }
}

module.exports = EchoCharacteristic;
