const util = require('util');

const bleno = require('../..');
const BlenoPrimaryService = bleno.PrimaryService;

const SerialNumberCharacteristic = require('./serial-number-characteristic');
const HardwareRevisionCharacteristic = require('./hardware-revision-characteristic');

function DeviceInformationService (blink1) {
  DeviceInformationService.super_.call(this, {
    uuid: '180a',
    characteristics: [
      new SerialNumberCharacteristic(blink1),
      new HardwareRevisionCharacteristic(blink1)
    ]
  });
}

util.inherits(DeviceInformationService, BlenoPrimaryService);

module.exports = DeviceInformationService;
