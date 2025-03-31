const { withBindings } = require('../../');

const blenoMac = withBindings('mac');

const blenoUart = withBindings('hci', { hciDriver: 'uart' });

blenoMac.on('stateChange', state => {
  console.log('[MAC] on -> stateChange: ' + state);

  if (state === 'poweredOn') {
    blenoMac.stopAdvertising();
    blenoMac.startAdvertising('test', ['a2744045-7004-4da9-8ed3-6d2d9a208c0a']);
  } else {
    blenoMac.stopAdvertising();
  }
});

blenoUart.on('stateChange', state => {
    console.log('[UART] on -> stateChange: ' + state);
  
    if (state === 'poweredOn') {
      blenoUart.stopAdvertising();
      blenoUart.startAdvertising('test', ['a2744045-7004-4da9-8ed3-6d2d9a208c0a']);
    } else {
      blenoUart.stopAdvertising();
    }
  });

blenoMac.on('advertisingStart', error => {
  console.log('[MAC] on -> advertisingStart: ' + (error ? 'error ' + error : 'success'));
});

blenoUart.on('advertisingStart', error => {
    console.log('[UART] on -> advertisingStart: ' + (error ? 'error ' + error : 'success'));
});
