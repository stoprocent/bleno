const os = require('os');
const platform = os.platform();

const Bleno = require('./bleno');

function loadBindings (bindingType = null, options = {}) {
  switch (bindingType) {
    case 'hci':
      return new (require('./hci-socket/bindings'))(options);
    case 'mac':
      return new (require('./mac/bindings'))(options);
    case 'win':
      return new (require('./win/bindings'))(options);
    default:
      throw new Error('Unsupported binding type: ' + bindingType);
  }
}

// Default binding selection logic
function getDefaultBindings (options = {}) {
  if (
    platform === 'linux' ||
    platform === 'freebsd' || 
    platform === 'android' ||
    process.env.BLUETOOTH_HCI_SOCKET_UART_PORT ||
    process.env.BLUETOOTH_HCI_SOCKET_FORCE_UART
  ) {
    return loadBindings('hci', options);
  } else if (platform === 'darwin') {
    return loadBindings('mac', options);
  } else if (platform === 'win32') {
    return loadBindings('win', options);
  } else {
    throw new Error('Unsupported platform ' + platform);
  }
}

module.exports = function (bindingType = 'default', options = {}) {
  if (bindingType === 'default') {
    return new Bleno(getDefaultBindings(options));
  }
  return new Bleno(loadBindings(bindingType, options));
};
