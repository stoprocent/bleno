const EventEmitter = require('events');
const should = require('should');

const Bleno = require('../lib/bleno');

class MockBindings extends EventEmitter {
  setServices () {
  }
}

describe('Bleno', function () {
  it('should deliver service setup errors to the callback and error event', function (done) {
    const bindings = new MockBindings();
    const bleno = new Bleno(bindings);
    const expectedError = new Error('service setup failed');
    let emittedError;

    bleno.once('servicesSetError', (error) => {
      emittedError = error;
    });
    bleno.setServices([], (error) => {
      should(error).equal(expectedError);
      should(emittedError).equal(expectedError);
      done();
    });

    bindings.emit('servicesSet', expectedError);
  });
});
