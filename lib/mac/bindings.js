const events = require('events');
const util = require('util');

const { resolve } = require('path');
const dir = resolve(__dirname, '..', '..');
const binding = require('node-gyp-build')(dir);

const { BlenoMac } = binding;

util.inherits(BlenoMac, events.EventEmitter);

module.exports = new BlenoMac();
