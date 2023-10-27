const bleno = require('../..');
const BlenoPrimaryService = bleno.PrimaryService;
const BatteryLevelCharacteristic = require('./battery-level-characteristic');

class BatteryService extends BlenoPrimaryService {
  constructor () {
    super({
      uuid: '180F',
      characteristics: [
        new BatteryLevelCharacteristic()
      ]
    });
  }
}

module.exports = BatteryService;
