const Blink1 = require('node-blink1');

const bleno = require('../..');

const DeviceInformationService = require('./device-information-service');
const Blink1Service = require('./blink1-service');

const blink1 = new Blink1();

const deviceInformationService = new DeviceInformationService(blink1);
const blink1Service = new Blink1Service(blink1);

bleno.on('stateChange', function (state) {
  console.log('on -> stateChange: ' + state);

  if (state === 'poweredOn') {
    bleno.startAdvertising('blink1', [blink1Service.uuid]);
  } else {
    bleno.stopAdvertising();
  }
});

bleno.on('advertisingStart', function (error) {
  console.log('on -> advertisingStart: ' + (error ? 'error ' + error : 'success'));

  if (!error) {
    bleno.setServices([
      deviceInformationService,
      blink1Service
    ]);
  }
});
