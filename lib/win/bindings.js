const events = require('events');
const util = require('util');

const { resolve } = require('path');
const dir = resolve(__dirname, '..', '..');
console.log(dir);
const binding = require('node-gyp-build')(dir);

const { BlenoWinRT } = binding;

util.inherits(BlenoWinRT, events.EventEmitter);

module.exports = BlenoWinRT;