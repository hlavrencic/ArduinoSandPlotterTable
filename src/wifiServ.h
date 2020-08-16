#ifndef H_WIFISERV
    #define H_WIFISERV

    #include <DNSServer.h>
    #include <WiFi.h>
    #include <AsyncTCP.h>
    #include "ESPAsyncWebServer.h"
    #include <SPIFFS.h>
    #include <ArduinoJson.h>

    class RedirectInvalidHostHandler : public AsyncWebHandler {
    public:
        RedirectInvalidHostHandler() {}
        virtual ~RedirectInvalidHostHandler() {}

        bool canHandle(AsyncWebServerRequest *request){   
            auto header = request->getHeader("Host");
            if(!header){
            return true;
            }

            return header->value().compareTo("connectivitycheck.gstatic.com")!=0;
        }

        void handleRequest(AsyncWebServerRequest *request) {
        if(request->method() == HTTP_GET) {
        request->redirect("http://connectivitycheck.gstatic.com/generate_204");
        } else {
        request->send(404);
        }    
    }
    };

    class SPIFFSReadHandler : public AsyncWebHandler {
    public:
        SPIFFSReadHandler() {}
        virtual ~SPIFFSReadHandler() {}

        bool canHandle(AsyncWebServerRequest *request){
            if(request->method() != HTTP_GET) return false; // solo captura los HttpGet

            //Verifica el origen de la llamada:
            auto header = request->getHeader("Host");
            if(!header)return false;

            auto origenCorrecto = header->value().compareTo("connectivitycheck.gstatic.com")==0;
            if(!origenCorrecto) return false;

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

    typedef std::function<void(StaticJsonDocument<200> doc)> TextReceivedHandler;
    typedef std::function<void()> SocketStatusHandler;

    class WifiServ
    {
    public:
        WifiServ();
        void setup();
        void loop();
        void sendJson(StaticJsonDocument<200> doc);
        AsyncWebServer server = AsyncWebServer(80);
        AsyncWebSocket ws = AsyncWebSocket("/ws");
        TextReceivedHandler textReceivedHandler;
        SocketStatusHandler connectedHandler;
        SocketStatusHandler disconnectedHandler;
    private:
        void serilize(char* data);
        void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
        DNSServer _dNSServer;
    };

    WifiServ::WifiServ()
    {
    };
    
    void WifiServ::sendJson(StaticJsonDocument<200> doc){
        char* txt;
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

    void WifiServ::setup(){
        if(!SPIFFS.begin()){
            Serial.println("SPIFFS Mount Failed");
            return;
        }

        listDir("/");

        WiFi.softAP("MesitaDeArena");
        _dNSServer.start(53, "*", WiFi.softAPIP());

        server.addHandler(new RedirectInvalidHostHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
        server.addHandler(new SPIFFSReadHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
        server.on(
            "/generate_204",
            HTTP_GET,
            [](AsyncWebServerRequest * request){
            request->send(SPIFFS, "/index.html", "text/html");
            });  

        ws.onEvent([&](AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
            onEvent(server,client,type,arg,data,len);
        });
        server.addHandler(&ws);

        server.begin();        
    };

    void WifiServ::loop(){
        _dNSServer.processNextRequest();
    };

#endif