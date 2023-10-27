const bleno = require('../..');

const BlenoPrimaryService = bleno.PrimaryService;

const EchoCharacteristic = require('./characteristic');

console.log('bleno - echo');

bleno.on('stateChange', function (state) {
  console.log('on -> stateChange: ' + state);

  if (state === 'poweredOn') {
    bleno.setAddress('11:22:44:55:99:10');
    bleno.startAdvertising('echo', ['ec00']);
  } else {
    bleno.stopAdvertising();
  }
});

bleno.on('advertisingStart', function (error) {
  console.log('on -> advertisingStart: ' + (error ? 'error ' + error : 'success'));

  if (!error) {
    bleno.setServices([
      new BlenoPrimaryService({
        uuid: 'ec00',
        characteristics: [
          new EchoCharacteristic()
        ]
      })
    ]);
  }
});
