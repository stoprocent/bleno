const bleno = require('../..');

const BlenoPrimaryService = bleno.PrimaryService;

const EchoCharacteristic = require('./characteristic');

console.log('bleno - echo');

async function run() {
  try {
    await bleno.waitForPoweredOnAsync();
    console.log("Powered on");
    await bleno.setAddressAsync('11:22:44:55:99:77');
    console.log("Address set");
    await bleno.startAdvertisingAsync('echo', ['ec00']);
    console.log("Advertising started");
    await bleno.setServicesAsync([
    new BlenoPrimaryService({
        uuid: 'ec00',
        characteristics: [
          new EchoCharacteristic()
        ]
      })
    ]);
    console.log("Services set");
  } catch (error) {
    console.error("Error: ", error);
  }
}

bleno.on('accept', async (address) => {
  console.log("Accepted: ", address);
  await bleno.startAdvertisingAsync('echo', ['ec00']);
  console.log("Advertising started");
});

run();
