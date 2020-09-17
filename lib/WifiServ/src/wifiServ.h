#ifndef H_WIFISERV
    #define H_WIFISERV

    #include <Arduino.h>
    #include <DNSServer.h>
    #include <ESPAsyncWebServer.h>
    #include <ArduinoJson.h>

    #ifdef ARDUINO_ARCH_ESP32
        #include <WiFi.h>
        #include <AsyncTCP.h>
    #else
        #include <ESP8266WiFi.h>
        #include <ESPAsyncTCP.h>
    #endif

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

#endif