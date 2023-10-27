const bleno = require('../..');
const BlenoCharacteristic = bleno.Characteristic;
const BlenoDescriptor = bleno.Descriptor;

class HardwareRevisionCharacteristic extends BlenoCharacteristic {
  constructor (blink1) {
    super({
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

  onReadRequest (offset, callback) {
    if (offset) {
      callback(this.RESULT_ATTR_NOT_LONG, null);
    } else {
      this.blink1.version((version) => {
        callback(this.RESULT_SUCCESS, Buffer.from(version));
      });
    }
  }
}

module.exports = HardwareRevisionCharacteristic;
