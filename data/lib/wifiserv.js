class WifiServ {
  _log;
  _ws;
  _subscribers = [];
  _status;
  _connected = false;

  constructor() {
    let self = this;

    self._log = msg => {
      console.log(msg);
    };
  }

  initWebSocket(url) {
    let self = this;

    self._ws = new WebSocket(url);

    self._ws.onmessage = function(evt) {
      let obj;
      try {
        obj = JSON.parse(evt.data);
      } catch (e) {
        obj = e;
      }

      self._subscribers.forEach(readResponse => readResponse(obj));
    };

    let promesa = new Promise((resolve, reject) => {
      self._ws.onopen = function(evt) {
        if (self._status) self._status("OPENED");
        resolve();
        self._connected = true;
      };
    });

    self._ws.onclose = function(evt) {
      if (self._status) self._status("CLOSED");
      self._connected = false;
      setTimeout(() => self.initWebSocket(url), 1000);
    };

    self._ws.onerror = function(evt) {
      if (self._status) self._status("ERROR");
      self._connected = false;
      throw evt;
    };

    return promesa;
  }

  subscribe(readResponse) {
    this._subscribers.push(readResponse);
  }

  send(obj) {
    // Espero a que inicialice el socket, antes de enviar algo.
    if (!this._connected) {
      console.warn("Esperando inicio socket.");
      setTimeout(() => this.send(obj), 500);
      return;
    }

    let txt = JSON.stringify(obj);
    this._ws.send(txt);
  }
}

if (typeof module !== "undefined" && typeof module.exports !== "undefined")
  module.exports = WifiServ;

define(["configParams"], configParams => {
  let wifiServ = new WifiServ();
  wifiServ.initWebSocket(configParams.socketURL);
  return wifiServ;
});
