const util = require('util');
const bleno = require('../..');

const PizzaCrustCharacteristic = require('./pizza-crust-characteristic');
const PizzaToppingsCharacteristic = require('./pizza-toppings-characteristic');
const PizzaBakeCharacteristic = require('./pizza-bake-characteristic');

function PizzaService (pizza) {
  bleno.PrimaryService.call(this, {
    uuid: '13333333333333333333333333333337',
    characteristics: [
      new PizzaCrustCharacteristic(pizza),
      new PizzaToppingsCharacteristic(pizza),
      new PizzaBakeCharacteristic(pizza)
    ]
  });
}

util.inherits(PizzaService, bleno.PrimaryService);

module.exports = PizzaService;
