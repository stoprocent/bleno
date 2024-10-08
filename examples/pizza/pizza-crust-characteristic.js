const bleno = require('../..');
const { PizzaCrust } = require('./pizza');

class PizzaCrustCharacteristic extends bleno.Characteristic {
  constructor (pizza) {
    super({
      uuid: '13333333333333333333333333330001',
      properties: ['read', 'write'],
      descriptors: [
        new bleno.Descriptor({
          uuid: '2901',
          value: 'Gets or sets the type of pizza crust.'
        })
      ]
    });

    this.pizza = pizza;
  }

  onWriteRequest (data, offset, withoutResponse, callback) {
    if (offset) {
      callback(this.RESULT_ATTR_NOT_LONG);
    } else if (data.length !== 1) {
      callback(this.RESULT_INVALID_ATTRIBUTE_LENGTH);
    } else {
      const crust = data.readUInt8(0);
      switch (crust) {
        case PizzaCrust.NORMAL:
        case PizzaCrust.DEEP_DISH:
        case PizzaCrust.THIN:
          this.pizza.crust = crust;
          callback(this.RESULT_SUCCESS);
          break;
        default:
          callback(this.RESULT_UNLIKELY_ERROR);
          break;
      }
    }
  }

  onReadRequest (offset, callback) {
    if (offset) {
      callback(this.RESULT_ATTR_NOT_LONG, null);
    } else {
      const data = Buffer.alloc(1);
      data.writeUInt8(this.pizza.crust, 0);
      callback(this.RESULT_SUCCESS, data);
    }
  }
}

module.exports = PizzaCrustCharacteristic;
