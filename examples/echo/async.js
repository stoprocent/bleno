const { withBindings } = require('../../');

const bleno = withBindings('default');
// const bleno = withBindings('mac');
// const bleno = withBindings('hci', { hciDriver: 'uart' });

const BlenoPrimaryService = bleno.PrimaryService;

const EchoCharacteristic = require('./characteristic');

console.log('bleno - echo');

async function run () {
  try {
    await bleno.waitForPoweredOnAsync();
    console.log("Powered on");
    await bleno.setAddressAsync('11:22:44:55:99:77');
    console.log("Address set");
    await bleno.startAdvertisingAsync('echo-async', ['ec00']);
    console.log("Advertising started");
    await bleno.setServicesAsync([
    new BlenoPrimaryService ({
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
  // We are not stopping advertising here because we want to allow another central to be able to connect
  // await bleno.stopAdvertisingAsync();
});

bleno.on('disconnect', async (address) => {
  console.log("Disconnected: ", address);
  // We are not starting advertising here because we didnt stop advertising in the accept event
  // await bleno.startAdvertisingAsync('echo-async', ['ec00']);
});

run();
