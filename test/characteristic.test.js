/* jshint mocha: true */

const should = require('should');

const Characteristic = require('../lib/characteristic');
Characteristic.RESULT_SUCCESS = 0x99; // override default value as 0x00 isnt the best value for this test

describe('Characteristic', function () {
  const mockUuid = 'mockuuid';
  const mockProperties = ['property1', 'property2', 'property3'];
  const mockSecure = ['secure1', 'secure2', 'secure3'];
  const mockValue = Buffer.from('mock value');
  const mockDescriptors = [{}, {}, {}];

  const mockOnReadRequest = function () {};
  const mockOnWriteRequest = function () {};
  const mockOnSubscribe = function () {};
  const mockOnUnsubscribe = function () {};
  const mockOnNotify = function () {};
  const mockOnIndicate = function () {};

  const mockMaxValueSize = 20;
  const mockUpdateValueCallback = function () {};

  it('should create with uuid option', function () {
    const characteristic = new Characteristic({
      uuid: mockUuid
    });

    characteristic.uuid.should.equal(mockUuid);

    Array.isArray(characteristic.properties).should.equal(true);
    characteristic.properties.length.should.equal(0);

    Array.isArray(characteristic.secure).should.equal(true);
    characteristic.secure.length.should.equal(0);

    should(characteristic.value).equal(null);

    Array.isArray(characteristic.descriptors).should.equal(true);
    characteristic.descriptors.length.should.equal(0);
  });

  it('should create with properties option', function () {
    const characteristic = new Characteristic({
      properties: mockProperties
    });

    characteristic.properties.should.equal(mockProperties);
  });

  it('should create with secure option', function () {
    const characteristic = new Characteristic({
      secure: mockSecure
    });

    characteristic.secure.should.equal(mockSecure);
  });

  it('should create with value option', function () {
    const characteristic = new Characteristic({
      properties: ['read'],
      value: mockValue
    });

    characteristic.value.should.equal(mockValue);
  });

  it('should not create with value option and non-read properties', function () {
    (function () {
      // eslint-disable-next-line no-unused-vars
      const characteristic = new Characteristic({
        properties: ['write'],
        value: mockValue
      });
    }).should.throw();
  });

  it('should create with descriptors option', function () {
    const characteristic = new Characteristic({
      descriptors: mockDescriptors
    });

    characteristic.descriptors.should.equal(mockDescriptors);
  });

  it('should create with onReadRequest option', function () {
    const characteristic = new Characteristic({
      onReadRequest: mockOnReadRequest
    });

    characteristic.onReadRequest.should.equal(mockOnReadRequest);
  });

  it('should create with onWriteRequest option', function () {
    const characteristic = new Characteristic({
      onWriteRequest: mockOnWriteRequest
    });

    characteristic.onWriteRequest.should.equal(mockOnWriteRequest);
  });

  it('should create with onSubscribe option', function () {
    const characteristic = new Characteristic({
      onSubscribe: mockOnSubscribe
    });

    characteristic.onSubscribe.should.equal(mockOnSubscribe);
  });

  it('should create with onUnsubscribe option', function () {
    const characteristic = new Characteristic({
      onUnsubscribe: mockOnUnsubscribe
    });

    characteristic.onUnsubscribe.should.equal(mockOnUnsubscribe);
  });

  it('should create with onNotify option', function () {
    const characteristic = new Characteristic({
      onNotify: mockOnNotify
    });

    characteristic.onNotify.should.equal(mockOnNotify);
  });

  it('should create with onIndicate option', function () {
    const characteristic = new Characteristic({
      onIndicate: mockOnIndicate
    });

    characteristic.onIndicate.should.equal(mockOnIndicate);
  });

  it('should toString', function () {
    const characteristic = new Characteristic({
      uuid: mockUuid
    });

    characteristic.toString().should.equal('{"uuid":"mockuuid","properties":[],"secure":[],"value":null,"descriptors":[]}');
  });

  it('should handle read request', function (done) {
    const characteristic = new Characteristic({});
    const handle = 0;

    characteristic.emit('readRequest', handle, 0, function (result, data) {
      result.should.equal(Characteristic.RESULT_UNLIKELY_ERROR);
      should(data).equal(null);
      done();
    });
  });

  it('should handle write request', function (done) {
    const characteristic = new Characteristic({});
    const handle = 0;

    characteristic.emit('writeRequest', handle, Buffer.alloc(0), 0, false, function (result) {
      result.should.equal(Characteristic.RESULT_UNLIKELY_ERROR);
      done();
    });
  });

  it('should handle unsubscribe', function () {
    const characteristic = new Characteristic({});
    const handle = 0;

    characteristic._maxValueSizes.set(handle, mockMaxValueSize);
    characteristic._updateValueCallbacks.set(handle, mockUpdateValueCallback);

    characteristic.emit('unsubscribe', handle);
    
    should(characteristic.getMaxValueSize(handle)).equal(undefined);
    should(characteristic._maxValueSizes.has(handle)).equal(false);
    should(characteristic._updateValueCallbacks.has(handle)).equal(false);
  });

  describe('Handle-based updates', function () {
    it('should handle subscribe with different handles', function () {
      const characteristic = new Characteristic({});
      const handle1 = 1;
      const handle2 = 2;
      const maxValueSize1 = 20;
      const maxValueSize2 = 30;
      let updateCallback1Called = false;
      let updateCallback2Called = false;

      const updateCallback1 = (data) => {
        updateCallback1Called = true;
        should(data).equal('test1');
      };

      const updateCallback2 = (data) => {
        updateCallback2Called = true;
        should(data).equal('test2');
      };

      // Subscribe first client
      characteristic.emit('subscribe', handle1, maxValueSize1, updateCallback1);
      should(characteristic.getMaxValueSize(handle1)).equal(maxValueSize1);
      should(characteristic._updateValueCallbacks.has(handle1)).equal(true);
      should(characteristic._updateValueCallbacks.get(handle1)).equal(updateCallback1);

      // Subscribe second client
      characteristic.emit('subscribe', handle2, maxValueSize2, updateCallback2);
      should(characteristic.getMaxValueSize(handle2)).equal(maxValueSize2);
      should(characteristic._updateValueCallbacks.has(handle2)).equal(true);
      should(characteristic._updateValueCallbacks.get(handle2)).equal(updateCallback2);

      // Test updates for each handle
      characteristic._updateValueCallbacks.get(handle1)('test1');
      should(updateCallback1Called).equal(true);

      characteristic._updateValueCallbacks.get(handle2)('test2');
      should(updateCallback2Called).equal(true);
    });

    it('should handle unsubscribe for specific handle', function () {
      const characteristic = new Characteristic({});
      const handle1 = 1;
      const handle2 = 2;
      const maxValueSize = 20;

      // Subscribe both clients
      characteristic.emit('subscribe', handle1, maxValueSize, () => {});
      characteristic.emit('subscribe', handle2, maxValueSize, () => {});

      // Unsubscribe first client
      characteristic.emit('unsubscribe', handle1);
      
      // First client should be removed
      should(characteristic.getMaxValueSize(handle1)).equal(undefined);
      should(characteristic._updateValueCallbacks.has(handle1)).equal(false);
      
      // Second client should still be subscribed
      should(characteristic.getMaxValueSize(handle2)).equal(maxValueSize);
      should(characteristic._updateValueCallbacks.has(handle2)).equal(true);

      // Unsubscribe second client
      characteristic.emit('unsubscribe', handle2);
      
      // Both clients should be removed
      should(characteristic.getMaxValueSize(handle1)).equal(undefined);
      should(characteristic.getMaxValueSize(handle2)).equal(undefined);
      should(characteristic._updateValueCallbacks.has(handle1)).equal(false);
      should(characteristic._updateValueCallbacks.has(handle2)).equal(false);
    });

    it('should handle read requests for different handles', function (done) {
      const handle1 = 1;
      const handle2 = 2;

      let readCallback1Called = false;
      let readCallback2Called = false;

      const characteristic = new Characteristic({ 
        onReadRequest: (handle, offset, callback) => {
          if (handle === handle1) {
            readCallback1Called = true;
            callback(Characteristic.RESULT_SUCCESS, Buffer.from('test1'));
          } else if (handle === handle2) {
            readCallback2Called = true;
            callback(Characteristic.RESULT_SUCCESS, Buffer.from('test2'));
          }
        }
      });

      // Test read request for first handle
      characteristic.emit('readRequest', handle1, 0, (result, data) => {
        should(result).equal(Characteristic.RESULT_SUCCESS);
        should(data.toString()).equal('test1');
        should(readCallback1Called).equal(true);
      });

      // Test read request for second handle
      characteristic.emit('readRequest', handle2, 0, (result, data) => {
        should(result).equal(Characteristic.RESULT_SUCCESS);
        should(data.toString()).equal('test2');
        should(readCallback2Called).equal(true);
        done();
      });
    });

    it('should handle write requests for different handles', function (done) {
      const handle1 = 1;
      const handle2 = 2;

      const data1 = Buffer.from('test1');
      const data2 = Buffer.from('test2');

      let writeCallback1Called = false;
      let writeCallback2Called = false;

      const characteristic = new Characteristic({
        onWriteRequest: (handle, data, offset, withoutResponse, callback) => {
          if (handle === handle1) {
            writeCallback1Called = true;
            should(data).equal(data1);
            callback(Characteristic.RESULT_SUCCESS);
          } else if (handle === handle2) {
            writeCallback2Called = true;
            should(data).equal(data2);
            callback(Characteristic.RESULT_SUCCESS);
          }
        }
      });
      // Test write request for first handle
      characteristic.emit('writeRequest', handle1, data1, 0, false, (result) => {
        should(result).equal(Characteristic.RESULT_SUCCESS);
        should(writeCallback1Called).equal(true);
      });

      // Test write request for second handle
      characteristic.emit('writeRequest', handle2, data2, 0, false, (result) => {
        should(result).equal(Characteristic.RESULT_SUCCESS);
        should(writeCallback2Called).equal(true);
        done();
      });
    });
  });
});
