const bleno = require('../..');

const PizzaCrustCharacteristic = require('./pizza-crust-characteristic');
const PizzaToppingsCharacteristic = require('./pizza-toppings-characteristic');
const PizzaBakeCharacteristic = require('./pizza-bake-characteristic');

class PizzaService extends bleno.PrimaryService {
  constructor (pizza) {
    super({
      uuid: '13333333333333333333333333333337',
      characteristics: [
        new PizzaCrustCharacteristic(pizza),
        new PizzaToppingsCharacteristic(pizza),
        new PizzaBakeCharacteristic(pizza)
      ]
    });
  }
}

module.exports = PizzaService;
