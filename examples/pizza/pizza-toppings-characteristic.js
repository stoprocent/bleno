const bleno = require('../..');

class PizzaToppingsCharacteristic extends bleno.Characteristic {
  constructor (pizza) {
    super({
      uuid: '13333333333333333333333333330002',
      properties: ['read', 'write'],
      descriptors: [
        new bleno.Descriptor({
          uuid: '2901',
          value: 'Gets or sets the pizza toppings.'
        })
      ]
    });

    this.pizza = pizza;
  }

  onWriteRequest (data, offset, withoutResponse, callback) {
    if (offset) {
      callback(this.RESULT_ATTR_NOT_LONG);
    } else if (data.length !== 2) {
      callback(this.RESULT_INVALID_ATTRIBUTE_LENGTH);
    } else {
      this.pizza.toppings = data.readUInt16BE(0);
      callback(this.RESULT_SUCCESS);
    }
  }

  onReadRequest (offset, callback) {
    if (offset) {
      callback(this.RESULT_ATTR_NOT_LONG, null);
    } else {
      const data = Buffer.alloc(2);
      data.writeUInt16BE(this.pizza.toppings, 0);
      callback(this.RESULT_SUCCESS, data);
    }
  }
}

module.exports = PizzaToppingsCharacteristic;
