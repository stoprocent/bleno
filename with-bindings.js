const Bleno = require('./lib/bleno');

module.exports = function (bindings) {
  return new Bleno(bindings);
};
