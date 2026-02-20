const BlenoEventEmitter = require('../bleno-event-emitter');
const Smp = require('./smp');
const Mgmt = require('./mgmt');

class AclStream extends BlenoEventEmitter {
  constructor (hci, handle, localAddressType, localAddress, remoteAddressType, remoteAddress) {
    super();
    this._hci = hci;
    this._handle = handle;
    this.encypted = false;

    const mgmt = new Mgmt(hci.BluetoothHciSocket);
    this._smp = new Smp(this, mgmt, localAddressType, localAddress, remoteAddressType, remoteAddress);
  }

  close () {
    this.emit('end', this._handle);
    this._smp.close();
  }

  write (cid, data) {
    this._hci.queueAclDataPkt(this._handle, cid, data);
  }

  push (cid, data) {
    this.emit('data', this._handle, cid, data);
  }

  pushEncrypt (encrypt) {
    this.encrypted = !!encrypt;

    this.emit('encryptChange', this.encrypted);
  }

  pushLtkNegReply () {
    this.emit('ltkNegReply');
  }
}

module.exports = AclStream;
