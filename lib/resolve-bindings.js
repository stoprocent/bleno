const os = require('os');

module.exports = function (options = {}) {
  const platform = os.platform();

  if (
    platform === 'linux' ||
    platform === 'freebsd' ||
    platform === 'win32' ||
    platform === 'android' ||
    process.env.BLUETOOTH_HCI_SOCKET_UART_PORT ||
    process.env.BLUETOOTH_HCI_SOCKET_FORCE_UART) {
    return new (require('./hci-socket/bindings'))(options);
  } else if (platform === 'darwin') {
    return new (require('./mac/bindings'))(options);
  } else {
    throw new Error('Unsupported platform ' + platform);
  }
};
