const bleno = require('../../with-custom-binding')({
  bindParams: {
    uart: {
      port: '/dev/tty...',
      baudRate: 1000000
    }
  }
});

bleno.on('stateChange', state => {
  console.log('on -> stateChange: ' + state);

  if (state === 'poweredOn') {
    bleno.setAddress('11:22:33:44:55:66');
    bleno.startAdvertisingIBeacon('a2744045-7004-4da9-8ed3-6d2d9a208c0a', 1234, 5678);
  } else {
    bleno.stopAdvertising();
  }
});

bleno.on('advertisingStart', error => {
  console.log('on -> advertisingStart: ' + (error ? 'error ' + error : 'success'));
});
