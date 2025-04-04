const { EventEmitter } = require('events');

const { resolve } = require('path');
const dir = resolve(__dirname, '..', '..');
const binding = require('node-gyp-build')(dir);

const { BlenoMac } = binding;

Object.setPrototypeOf(BlenoMac.prototype, EventEmitter.prototype);

module.exports = BlenoMac;
