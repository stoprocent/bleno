const util = require('util');

const bleno = require('../..');

const BlenoPrimaryService = bleno.PrimaryService;

const BatteryLevelCharacteristic = require('./battery-level-characteristic');

function BatteryService () {
  BatteryService.super_.call(this, {
    uuid: '180F',
    characteristics: [
      new BatteryLevelCharacteristic()
    ]
  });
}

util.inherits(BatteryService, BlenoPrimaryService);

module.exports = BatteryService;
