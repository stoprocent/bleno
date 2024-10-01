const bleno = require('../../with-custom-binding')({
  bindParams: {
    uart: {
      port: '/dev/tty...', // Specify the path to the UART device
      baudRate: 1000000
    }
  }
});

function generateRandomBLEMacAddress () {
  // Create an array to store 6 bytes for the MAC address
  let macAddr = [];
  // Generate 6 random bytes
  for (let i = 0; i < 6; i++) {
      // Generate a random byte between 0x00 and 0xFF
      let randomByte = Math.floor(Math.random() * 256);      
      // Ensure the first byte is a locally administered unicast address
      if (i === 0) {
          randomByte = (randomByte & 0xFE) | 0x02; // Clear multicast bit and set locally administered bit
      }
      // Convert the byte to a two-character hexadecimal string
      macAddr.push(randomByte.toString(16).padStart(2, '0'));
  }
  // Join the array into a MAC address string with colons
  return macAddr.join(':');
}
let interval = null;
bleno.on('stateChange', state => {
  console.log('on -> stateChange: ' + state);

  if (state === 'poweredOn') {
    clearInterval(interval);
    interval = setInterval(() => {
      // Stop advertising
      bleno.stopAdvertising();
      // Set a random MAC address
      bleno.setAddress(generateRandomBLEMacAddress());
      // Start advertising as an iBeacon
      bleno.startAdvertisingIBeacon('a2744045-7004-4da9-8ed3-6d2d9a208c0a', 1234, 5678);
    }, 4000);
  } else {
    bleno.stopAdvertising();
  }
});

bleno.on('advertisingStart', error => {
  console.log('on -> advertisingStart: ' + (error ? 'error ' + error : 'success'));
});
