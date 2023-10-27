const bleno = require('../..');
const BlenoPrimaryService = bleno.PrimaryService;

const Blink1RGBCharacteristic = require('./blink1-rgb-characteristic');
const Blink1FadeRGBCharacteristic = require('./blink1-fade-rgb-characteristic');

class Blink1Service extends BlenoPrimaryService {
  constructor (blink1) {
    super({
      uuid: '01010101010101010101010101010101',
      characteristics: [
        new Blink1RGBCharacteristic(blink1),
        new Blink1FadeRGBCharacteristic(blink1)
      ]
    });
  }
}

module.exports = Blink1Service;
