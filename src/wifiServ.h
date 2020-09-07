#ifndef H_WIFISERV
    #define H_WIFISERV

    #include <Arduino.h>
    #include <DNSServer.h>
    #include <WiFi.h>
    #include <AsyncTCP.h>
    #include "ESPAsyncWebServer.h"
    #include <SPIFFS.h>
    #include <ArduinoJson.h>

    typedef std::function<void(StaticJsonDocument<200> doc)> TextReceivedHandler;
    typedef std::function<void()> SocketStatusHandler;

    enum WifiServEstadoConexion {NINGUNO = 0, CONFIGURADO = 1, CONECTADO = 2, CONECTADO_AP = 3};

    class WifiServ
    {
    public:
        void connect(const char* ssid, const char* pass = (const char*)__null);
        void connectAP(const char* ssid);
        void loop();
        void sendJson(StaticJsonDocument<200> doc);
        IPAddress GetIP();
        AsyncWebServer server = AsyncWebServer(80);
        AsyncWebSocket ws = AsyncWebSocket("/ws");
        TextReceivedHandler textReceivedHandler;
        SocketStatusHandler connectedHandler;
        SocketStatusHandler disconnectedHandler;
    private:
        void _connect(const char* ssid, const char* pass = (const char*)__null);
        void _setup();
        void _cleanWifi();
        void serilize(char* data);
        void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
        DNSServer _dNSServer;
        WifiServEstadoConexion _estadoConexion = WifiServEstadoConexion::NINGUNO;
        String _ssid;
        String _pass;
        IPAddress _ip;
        unsigned short _cont;
    };

    class RedirectInvalidHostHandler : public AsyncWebHandler {
    public:
        RedirectInvalidHostHandler(WifiServ* wifiServ) {
            _wifiServ = wifiServ;
        }
        virtual ~RedirectInvalidHostHandler() {}

        bool canHandle(AsyncWebServerRequest *request){   
            auto header = request->getHeader("Host");
            if(!header){
                return true;
            }

            auto hostValue = header->value();
            auto ip = _wifiServ->GetIP().toString();
            auto hostValido = hostValue.compareTo("connectivitycheck.gstatic.com")==0 || hostValue.compareTo(ip)==0;
            return !hostValido;
        }

        void handleRequest(AsyncWebServerRequest *request) {
            if(request->method() == HTTP_GET) {
                request->redirect("http://connectivitycheck.gstatic.com/generate_204");
            } else {
                request->send(404);
            }    
        }
    private:
        WifiServ* _wifiServ;
    };

    class SPIFFSRedirectIndexHandler : public AsyncWebHandler {
    public:
        SPIFFSRedirectIndexHandler() {}
        virtual ~SPIFFSRedirectIndexHandler() {}

        bool canHandle(AsyncWebServerRequest *request){
            if(request->method() != HTTP_GET) return false; // solo captura los HttpGet
            
            auto url = request->url();
            auto esIndex = url.compareTo("/") == 0;
            return esIndex;
        }

        void handleRequest(AsyncWebServerRequest *request) {
            request->redirect("/index.html");
        }
    };

    class SPIFFSReadHandler : public AsyncWebHandler {
    public:
        SPIFFSReadHandler() {}
        virtual ~SPIFFSReadHandler() {}

        bool canHandle(AsyncWebServerRequest *request){
            if(request->method() != HTTP_GET) return false; // solo captura los HttpGet

            // Verifica que el archivo exista
            auto url = request->url().c_str();
            auto file = SPIFFS.open(url);
            auto existe = file.available();
            file.close();
            return existe;
        }

        void handleRequest(AsyncWebServerRequest *request) {
            auto urlStr = request->url();
            auto contentType = _getType(urlStr);
            request->send(SPIFFS, urlStr, contentType);
        }
    private:
        bool _hasExt(String path, const char* ext){
            auto i = path.lastIndexOf(ext);
            auto hasExt = i > 0 && i >= path.length() - 5;
            return hasExt;
        }

        const char * _getType(String path){
            if(_hasExt(path, ".jpg")){
            return "image/jpeg";
            } 
            
            if(_hasExt(path, ".svg")){
            return "image/svg+xml";
            } 
            
            if(_hasExt(path, ".gif")){
            return "image/gif";
            } 
            
            if(_hasExt(path, ".png")){
            return "image/png";
            } 

            if(_hasExt(path, ".css")){
            return "text/css";
            } 

            if(_hasExt(path, ".html")){
            return "text/html";
            } 
            
            if(_hasExt(path, ".ico")){
            return "image/vnd.microsoft.icon";
            } 
            
            if(_hasExt(path, ".js")){
            return "text/javascript";
            } 

            return "text/plain";
        }
    };

    void listDir(const char * dirname){
        Serial.printf("Listing directory: %s\r\n", dirname);

        File root = SPIFFS.open(dirname);
        if(!root){
            Serial.println("- failed to open directory");
            return;
        }
        if(!root.isDirectory()){
            Serial.println(" - not a directory");
            return;
        }

        File file = root.openNextFile();
        while(file){
            if(file.isDirectory()){
                Serial.print("  DIR : ");
                Serial.println(file.name());
                listDir(file.name());
            } else {
                Serial.print("  FILE: ");
                Serial.print(file.name());
                Serial.print("\tSIZE: ");
                Serial.println(file.size());
            }
            file = root.openNextFile();
        }
    };
    
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

        if(!SPIFFS.begin()){
            Serial.println("SPIFFS Mount Failed");
            return;
        }

        listDir("/");
        
        server.addHandler(new RedirectInvalidHostHandler(this));
        server.addHandler(new SPIFFSReadHandler());
        server.addHandler(new SPIFFSRedirectIndexHandler());
        server.on(
            "/generate_204",
            HTTP_GET,
            [](AsyncWebServerRequest * request){
            request->send(SPIFFS, "/indexAP.html", "text/html");
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

#endif