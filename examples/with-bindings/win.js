const { withBindings, PrimaryService, Characteristic } = require('../../');

const bleno = withBindings('win');

bleno.on('stateChange', state => {
  console.log('[WIN] on -> stateChange: ' + state);

  if (state === 'poweredOn') {
    bleno.stopAdvertising();
    bleno.startAdvertising('test', ['a2744045-7004-4da9-8ed3-6d2d9a208c0a']);
  } else {
    bleno.stopAdvertising();
  }
});

bleno.on('advertisingStart', function (error) {
  console.log('on -> advertisingStart: ' + (error ? 'error ' + error : 'success'));
  setTimeout(() => {
  }, 40000);
  if (!error) {
    bleno.setServices([
      new PrimaryService({
        uuid: 'ec00',
        characteristics: [
          new Characteristic({
            uuid: 'ec01',
            properties: ['read'],
            value: 'Hello, world!',
          }),
        ],
      }),
    ]);
  }
});

setInterval(() => {
  console.log('Hello, world!');
}, 1000);

process.on('SIGINT', () => {
  bleno.stop();
  console.log('SIGINT');
  process.exit(0);
});
