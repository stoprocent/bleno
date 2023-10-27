const util = require('util');
const os = require('os');
const exec = require('child_process').exec;

const bleno = require('../..');

const Descriptor = bleno.Descriptor;
const Characteristic = bleno.Characteristic;

const BatteryLevelCharacteristic = function () {
  BatteryLevelCharacteristic.super_.call(this, {
    uuid: '2A19',
    properties: ['read'],
    descriptors: [
      new Descriptor({
        uuid: '2901',
        value: 'Battery level between 0 and 100 percent'
      }),
      new Descriptor({
        uuid: '2904',
        value: Buffer.from([0x04, 0x01, 0x27, 0xAD, 0x01, 0x00, 0x00]) // maybe 12 0xC unsigned 8 bit
      })
    ]
  });
};

util.inherits(BatteryLevelCharacteristic, Characteristic);

BatteryLevelCharacteristic.prototype.onReadRequest = function (offset, callback) {
  if (os.platform() === 'darwin') {
    // eslint-disable-next-line n/handle-callback-err
    exec('pmset -g batt', function (error, stdout, stderr) {
      const data = stdout.toString();
      // data - 'Now drawing from \'Battery Power\'\n -InternalBattery-0\t95%; discharging; 4:11 remaining\n'
      let percent = data.split('\t')[1].split(';')[0];
      console.log(percent);
      percent = parseInt(percent, 10);
      callback(this.RESULT_SUCCESS, Buffer.from([percent]));
    });
  } else {
    // return hardcoded value
    callback(this.RESULT_SUCCESS, Buffer.from([98]));
  }
};

module.exports = BatteryLevelCharacteristic;
