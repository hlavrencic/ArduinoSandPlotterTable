class WifiServ {

  _log;
  _ws;
  _subscribers = [];
  _status;

  constructor(){
    let self = this;

    self._log = ((msg) => { console.log(msg); });
  }

  initWebSocket(url){
    let self = this;

    self._ws = new WebSocket(url);
    
    self._ws.onmessage = function(evt) {
      let obj;
      try{
        obj = JSON.parse(evt.data);
      }catch(e){
        obj = e;      
      }

      self._subscribers.forEach(readResponse => readResponse(obj));
    };

    self._ws.onopen = function(evt) {
      if(self._status) self._status("OPENED");
    };

    self._ws.onclose = function(evt) {
      if(self._status) self._status("CLOSED");
    }; 
    
    self._ws.onerror = function(evt){
      if(self._status) self._status("ERROR");
      throw evt;
    };
  }

  subscribe(readResponse){
    this._subscribers.push(readResponse);
  }

  send(obj){
    let txt = JSON.stringify(obj);
    this._ws.send(txt);
  }

}

if (typeof module !== 'undefined' && typeof module.exports !== 'undefined')
module.exports = WifiServ;