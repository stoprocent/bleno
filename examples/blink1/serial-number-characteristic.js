const util = require('util');

const bleno = require('../..');

const BlenoCharacteristic = bleno.Characteristic;
const BlenoDescriptor = bleno.Descriptor;

function SerialNumberCharacteristic (blink1) {
  SerialNumberCharacteristic.super_.call(this, {
    uuid: '2a25',
    properties: ['read'],
    value: Buffer.from(blink1.serialNumber),
    descriptors: [
      new BlenoDescriptor({
        uuid: '2901',
        value: 'blink(1) serial number'
      })
    ]
  });
}

util.inherits(SerialNumberCharacteristic, BlenoCharacteristic);

module.exports = SerialNumberCharacteristic;
