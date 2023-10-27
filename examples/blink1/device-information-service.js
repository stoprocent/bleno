const bleno = require('../..');
const BlenoPrimaryService = bleno.PrimaryService;

const SerialNumberCharacteristic = require('./serial-number-characteristic');
const HardwareRevisionCharacteristic = require('./hardware-revision-characteristic');

class DeviceInformationService extends BlenoPrimaryService {
  constructor (blink1) {
    super({
      uuid: '180a',
      characteristics: [
        new SerialNumberCharacteristic(blink1),
        new HardwareRevisionCharacteristic(blink1)
      ]
    });
  }
}

module.exports = DeviceInformationService;
