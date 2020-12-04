let SOCKET_URL = "ws://" + window.location.host + ":81";

try{
    let wifiServ = new WifiServ();
    let viewModel = new Binder(wifiServ, ko).bind();
    ko.applyBindings(viewModel);
    wifiServ.initWebSocket(SOCKET_URL);

    let container = document.getElementById('container');
    
    let inputFile = document.querySelector("input[type=file]");
    let nextTag = document.getElementById("next");
    let svgRead = new SvgReader(container);
    svgRead.bindInput(inputFile);
    svgRead.bindRange(container);
    nextTag.addEventListener("click", () => {
      svgRead.moveNext();
    });

    let carrito = new Carrito(wifiServ, svgRead);
}catch(e){
    alert(e);
}
