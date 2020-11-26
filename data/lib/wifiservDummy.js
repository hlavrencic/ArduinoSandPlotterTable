class WifiservDummy {
  _subscribers = [];
  _status;
  _sendStatusSwitch = false;

  initWebSocket() {
    let self = this;
    let promesa = new Promise(resolve => {
      setTimeout(() => {
        resolve();
        self._status("OPENED");
      }, 1000);
    });

    setInterval(() => {
      if (!self._sendStatusSwitch) return;

      self.received({
        wifi: {
          status: "CONNECTED",
          ip: "192.168.1.101",
          gw: "192.168.1.1",
          mask: "255.255.0.0",
          reason: null,
          ssid: "NETWORK_1",
          channel: 5
        }
      });
    }, 5000);

    return promesa;
  }

  subscribe(readResponse) {
    this._subscribers.push(readResponse);
  }

  received(obj) {
    this._subscribers.forEach(readResponse => readResponse(obj));
  }

  send(obj) {
    let self = this;
    setTimeout(() => {}, 1000);
  }
}

define([], () => {
  let serv = new WifiservDummy();
  serv.initWebSocket();
  return serv;
});
