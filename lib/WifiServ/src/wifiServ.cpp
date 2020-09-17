#include <wifiServ.h>
#include <RedirectIndexHandler.h>
#include <RedirectInvalidHostHandler.h>
#include <SPIFFSReadHandler.h>
#include <FSInclude.h>

void WifiServ::sendJson(StaticJsonDocument<200> doc){
    char txt[200];
    serializeJson(doc, txt);
    ws.textAll(txt);
};

void WifiServ::serilize(char* data){
    if(!textReceivedHandler) return;

    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, data);

    // Test if parsing succeeds.
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    } 
    
    textReceivedHandler(doc);
};

void WifiServ::onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
    if(type == WS_EVT_CONNECT){
        
        if(connectedHandler) connectedHandler();
        Serial.println("Websocket client connection received");
    
    } else if(type == WS_EVT_DISCONNECT){
        if(disconnectedHandler) disconnectedHandler();

        Serial.println("Client disconnected");
    } 

    //data packet
    if(type != WS_EVT_DATA) return;

    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    if(info->final && info->index == 0 && info->len == len){
        //the whole message is in a single frame and we got all of it's data
        //os_printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);
        if(info->opcode == WS_TEXT){
            data[len] = 0;
            auto dataChar = (char*)data;

            serilize(dataChar);
        }
    }      
};

void WifiServ::connect(const char* ssid, const char* pass){
    _ssid = String(ssid);
    _pass = String(pass);
}

IPAddress WifiServ::GetIP(){
    return _ip;
}

void WifiServ::_connect(const char* ssid, const char* pass){
    
    _cleanWifi();

    WiFi.begin(ssid, pass);
    WiFi.setAutoReconnect(true);
    
    _estadoConexion = WifiServEstadoConexion::CONECTADO;

    auto status = WiFi.status();
    _cont = 0;
    while (status != WL_CONNECTED) {
        Serial.print("Connecting to WiFi... ");
        Serial.print(ssid);
        Serial.print("  STATUS: ");
        Serial.println(status);
        delay(500);
        status = WiFi.status();

        _cont++;
        if(_cont >= 20){
            Serial.println("CONNECTION FAILED.");
            connectAP("MesitaArena");
            return;
        }
    }

    _ip = WiFi.localIP();
    Serial.print("IP: ");
    Serial.println(_ip);
    
    server.begin();
}

void WifiServ::connectAP(const char* ssid){
    _cleanWifi();

    WiFi.softAP(ssid);
    

    _ip = WiFi.softAPIP();
    Serial.print("IP: ");
    Serial.println(_ip);
    if(!_dNSServer.start(53, "*", _ip)){
        Serial.println("Fallo DNS");
        return;
    }

    _setup();

    server.begin();

    _estadoConexion = WifiServEstadoConexion::CONECTADO_AP;
}

void WifiServ::_cleanWifi(){
    if(!(_estadoConexion == WifiServEstadoConexion::CONECTADO || _estadoConexion == WifiServEstadoConexion::CONECTADO_AP))
        return;
    
    auto estadoOriginal = _estadoConexion;
    _estadoConexion = WifiServEstadoConexion::CONFIGURADO;
    
    ws.closeAll();
    Serial.println("Server stop...");
    //server.reset();
    server.end();
    Serial.println("STOPPIN DNS...");
    _dNSServer.stop();

    if(estadoOriginal == WifiServEstadoConexion::CONECTADO_AP){
        Serial.println("disconecting AP...");
        WiFi.softAPdisconnect(true);
        
        delay(1000);
    } else if (estadoOriginal == WifiServEstadoConexion::CONECTADO){
        Serial.println("Disconecting WiFi...");
        WiFi.disconnect(true);
        delay(1000);
    }

}

void WifiServ::_setup(){
    if(_estadoConexion != WifiServEstadoConexion::NINGUNO){
        return;
    }

    if(!FSInclude.begin()){
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    SPIFFSReadHandler::listDir("/");
    
    server.addHandler(new RedirectInvalidHostHandler(this));
    server.addHandler(new SPIFFSReadHandler());
    server.addHandler(new RedirectIndexHandler());
    server.on(
        "/generate_204",
        HTTP_GET,
        [](AsyncWebServerRequest * request){
        request->send(FSInclude, "/indexAP.html", "text/html");
        });  
    
    ws.onEvent([&](AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
        onEvent(server,client,type,arg,data,len);
    });

    server.addHandler(&ws);
            
    _estadoConexion = WifiServEstadoConexion::CONFIGURADO;
};

void WifiServ::loop(){
    if(_ssid.length() > 0){
        _connect(_ssid.c_str(), _pass.c_str());
        _ssid.clear();
    }

    if(_estadoConexion != WifiServEstadoConexion::CONECTADO_AP) return;
    _dNSServer.processNextRequest();
};