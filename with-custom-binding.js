module.exports = function (options) {
  const Bleno = require('./lib/bleno');
  const bindings = require('./lib/resolve-bindings')(options);

  return new Bleno(bindings);
};
