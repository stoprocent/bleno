const BlenoEventEmitter = require('../bleno-event-emitter');

const { resolve } = require('path');
const dir = resolve(__dirname, '..', '..');
const binding = require('node-gyp-build')(dir);

const { BlenoWinRT } = binding;

Object.setPrototypeOf(BlenoWinRT.prototype, BlenoEventEmitter.prototype);

module.exports = BlenoWinRT;
