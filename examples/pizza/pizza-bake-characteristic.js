const bleno = require('../..');

class PizzaBakeCharacteristic extends bleno.Characteristic {
  constructor (pizza) {
    super({
      uuid: '13333333333333333333333333330003',
      properties: ['notify', 'write'],
      descriptors: [
        new bleno.Descriptor({
          uuid: '2901',
          value: 'Bakes the pizza and notifies when done baking.'
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
      const temperature = data.readUInt16BE(0);
      const self = this;
      this.pizza.once('ready', function (result) {
        if (self.updateValueCallback) {
          const responseData = Buffer.alloc(1);
          responseData.writeUInt8(result, 0);
          self.updateValueCallback(responseData);
        }
      });
      this.pizza.bake(temperature);
      callback(this.RESULT_SUCCESS);
    }
  }
}

module.exports = PizzaBakeCharacteristic;
