/* jshint mocha: true */
const PrimaryService = require('../lib/primary-service');

describe('PrimaryService', function () {
  const mockUuid = 'mockuuid';
  const mockCharacteristics = [{}, {}, {}];

  it('should create with uuid option', function () {
    const service = new PrimaryService({
      uuid: mockUuid
    });

    service.uuid.should.equal(mockUuid);

    Array.isArray(service.characteristics).should.equal(true);
    service.characteristics.length.should.equal(0);
  });

  it('should create with characteristics option', function () {
    const service = new PrimaryService({
      characteristics: mockCharacteristics
    });

    service.characteristics.should.equal(mockCharacteristics);
  });

  it('should toString', function () {
    const service = new PrimaryService({
      uuid: mockUuid
    });

    service.toString().should.equal('{"uuid":"mockuuid","characteristics":[]}');
  });
});
