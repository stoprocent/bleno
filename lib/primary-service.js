const NobleEventEmitter = require('./noble-event-emitter');
const UuidUtil = require('./uuid-util');

class PrimaryService extends NobleEventEmitter {
  constructor (options) {
    super();
    this.uuid = UuidUtil.removeDashes(options.uuid);
    this.characteristics = options.characteristics || [];
  }

  toString () {
    return JSON.stringify({
      uuid: this.uuid,
      characteristics: this.characteristics
    });
  }
}

module.exports = PrimaryService;
