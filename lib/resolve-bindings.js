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
    return require('./hci-socket/bindings');
  } else if (platform === 'darwin') {
    return require('./mac/bindings');
  } else {
    throw new Error('Unsupported platform ' + platform);
  }
};
