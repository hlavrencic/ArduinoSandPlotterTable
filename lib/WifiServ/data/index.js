let SOCKET_URL = "ws://" + window.location.host + "/ws";

try{
    let wifiServ = new WifiServ();
    let viewModel = new Binder(wifiServ, ko).bind();
    ko.applyBindings(viewModel);
    wifiServ.initWebSocket(SOCKET_URL);
}catch(e){
    alert(e);
}
