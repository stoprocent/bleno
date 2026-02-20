const NobleEventEmitter = require('../noble-event-emitter');

const { resolve } = require('path');
const dir = resolve(__dirname, '..', '..');
const binding = require('node-gyp-build')(dir);

const { BlenoMac } = binding;

Object.setPrototypeOf(BlenoMac.prototype, NobleEventEmitter.prototype);

module.exports = BlenoMac;
