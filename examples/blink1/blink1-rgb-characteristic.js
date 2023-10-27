const bleno = require('../..');
const BlenoCharacteristic = bleno.Characteristic;
const BlenoDescriptor = bleno.Descriptor;

class Blink1RGBCharacteristic extends BlenoCharacteristic {
  constructor (blink1) {
    super({
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

  onWriteRequest (data, offset, withoutResponse, callback) {
    if (offset) {
      callback(this.RESULT_ATTR_NOT_LONG);
    } else if (data.length !== 3) {
      callback(this.RESULT_INVALID_ATTRIBUTE_LENGTH);
    } else {
      const r = data.readUInt8(0);
      const g = data.readUInt8(1);
      const b = data.readUInt8(2);

      this.blink1.setRGB(r, g, b, () => {
        callback(this.RESULT_SUCCESS);
      });
    }
  }
}

module.exports = Blink1RGBCharacteristic;
