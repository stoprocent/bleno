const bleno = require('../..');
const BlenoCharacteristic = bleno.Characteristic;
const BlenoDescriptor = bleno.Descriptor;

class SerialNumberCharacteristic extends BlenoCharacteristic {
  constructor (blink1) {
    super({
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
}

module.exports = SerialNumberCharacteristic;
