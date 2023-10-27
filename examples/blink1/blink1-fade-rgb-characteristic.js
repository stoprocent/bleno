const bleno = require('../..');
const BlenoCharacteristic = bleno.Characteristic;
const BlenoDescriptor = bleno.Descriptor;

class Blink1FaceRGBCharacteristic extends BlenoCharacteristic {
  constructor (blink1) {
    super({
      uuid: '01010101010101010166616465524742',
      properties: ['write', 'writeWithoutResponse', 'notify'],
      descriptors: [
        new BlenoDescriptor({
          uuid: '2901',
          value: 'fade blink(1) RGB value'
        })
      ]
    });

    this.blink1 = blink1;
  }

  onWriteRequest (data, offset, withoutResponse, callback) {
    if (offset) {
      callback(this.RESULT_ATTR_NOT_LONG);
    } else if (data.length !== 5) {
      callback(this.RESULT_INVALID_ATTRIBUTE_LENGTH);
    } else {
      const fadeMillis = data.readUInt16LE(0);
      const r = data.readUInt8(2);
      const g = data.readUInt8(3);
      const b = data.readUInt8(4);

      this.blink1.fadeToRGB(fadeMillis, r, g, b, () => {
        if (this.updateValueCallback) {
          this.updateValueCallback(Buffer.from([r, g, b]));
        }
        callback(this.RESULT_SUCCESS);
      });
    }
  }
}

module.exports = Blink1FaceRGBCharacteristic;
