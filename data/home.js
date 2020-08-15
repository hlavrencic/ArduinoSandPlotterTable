   
function print(msg){
    alert(msg);
}

let wsUrl = "ws://" + window.location.host + "/ws";
print(wsUrl);
let ws  = new WebSocket(wsUrl);

try	{
    

    ws.onopen = function(evt) { 
        print("Conectado!");
        ws.send("Hello WebSockets!");
    };

    ws.onmessage = function(evt) {
        print("Received Message: " + evt.data);
    };

    ws.onclose = function(evt) {
        print("Connection closed.");
    }; 
    
    ws.onerror = function(evt){
        print("Error" + evt);
    };

}catch(e){
    print(e);
    throw e;
}

function ajaxSend(){
    var http = new XMLHttpRequest();
    var url = ".\\";
    http.open("POST", url, true);
    http.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
    http.onreadystatechange = function() {
        if(http.readyState == 4 && http.status == 200) { 
        //aqui obtienes la respuesta de tu peticion
        print(http.responseText);
        }
    }
    http.send(JSON.stringify({msg:'ojo'}));
}

function getFormData($form){
    let unindexed_array = $form.serializeArray();
    let indexed_array = {};

    $.map(unindexed_array, function(n, i){
        indexed_array[n['name']] = n['value'];
    });

    let strJson = JSON.stringify(indexed_array);
    return  strJson;
}

let $form = $( "form" );
$form.on( "submit", function( event ) {
    event.preventDefault();
    let s = getFormData($form);
    ws.send(s);
});
