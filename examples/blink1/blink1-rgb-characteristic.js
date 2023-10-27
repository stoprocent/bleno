const util = require('util');

const bleno = require('../..');
const BlenoCharacteristic = bleno.Characteristic;
const BlenoDescriptor = bleno.Descriptor;

function Blink1RGBCharacteristic (blink1) {
  Blink1RGBCharacteristic.super_.call(this, {
    uuid: '01010101010101010101010101524742',
    properties: ['write', 'writeWithoutResponse'],
    descriptors: [
      new BlenoDescriptor({
        uuid: '2901',
        value: 'set blink(1) RGB value'
      })
    ]
  });

  this.blink1 = blink1;
}

util.inherits(Blink1RGBCharacteristic, BlenoCharacteristic);

Blink1RGBCharacteristic.prototype.onWriteRequest = function (data, offset, withoutResponse, callback) {
  if (offset) {
    callback(this.RESULT_ATTR_NOT_LONG);
  } else if (data.length !== 3) {
    callback(this.RESULT_INVALID_ATTRIBUTE_LENGTH);
  } else {
    const r = data.readUInt8(0);
    const g = data.readUInt8(1);
    const b = data.readUInt8(2);

    this.blink1.setRGB(r, g, b, function () {
      callback(this.RESULT_SUCCESS);
    }.bind(this));
  }
};

module.exports = Blink1RGBCharacteristic;
