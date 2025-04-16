const { EventEmitter } = require('events');
const Smp = require('./smp');

class AclStream extends EventEmitter {
  constructor (hci, handle, localAddressType, localAddress, remoteAddressType, remoteAddress) {
    super();
    this._hci = hci;
    this._handle = handle;
    this.encypted = false;

    this._smp = new Smp(this, localAddressType, localAddress, remoteAddressType, remoteAddress);
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
