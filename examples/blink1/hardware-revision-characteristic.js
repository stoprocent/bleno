const util = require('util');

const bleno = require('../..');
const BlenoCharacteristic = bleno.Characteristic;
const BlenoDescriptor = bleno.Descriptor;

function HardwareRevisionCharacteristic (blink1) {
  HardwareRevisionCharacteristic.super_.call(this, {
    uuid: '2a27',
    properties: ['read'],
    descriptors: [
      new BlenoDescriptor({
        uuid: '2901',
        value: 'blink(1) version'
      })
    ]
  });

  this.blink1 = blink1;
}

util.inherits(HardwareRevisionCharacteristic, BlenoCharacteristic);

HardwareRevisionCharacteristic.prototype.onReadRequest = function (offset, callback) {
  if (offset) {
    callback(this.RESULT_ATTR_NOT_LONG, null);
  } else {
    this.blink1.version(function (version) {
      callback(this.RESULT_SUCCESS, Buffer.from(version));
    }.bind(this));
  }
};

module.exports = HardwareRevisionCharacteristic;
