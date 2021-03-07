#ifndef H_WifiConnect
    #define H_WifiConnect
    #include <Arduino.h>
    #include <functional>
    #include "WiFi.h"

    typedef std::function<void(IPAddress ip)> OnConnectEvent;
    typedef std::function<void()> OnRejectEvent;

    class WifiConnect{
    public:
        WifiConnect(OnConnectEvent connect, OnRejectEvent reject){
            cEvt = connect;
            rEvt = reject;
            
            WiFi.onEvent(
                [&](WiFiEvent_t event, WiFiEventInfo_t info)
                {
                    Serial.println("GOT IP!");

                    auto ip = WiFi.localIP();
                    cEvt(ip);
                }, SYSTEM_EVENT_STA_GOT_IP);

            WiFi.onEvent(
                [&](WiFiEvent_t event, WiFiEventInfo_t info)
                {
                    Serial.println("Connected to AP!");
                }, SYSTEM_EVENT_STA_CONNECTED);
            
            WiFi.onEvent(
                [&](WiFiEvent_t event, WiFiEventInfo_t info)
                {
                    WiFi.disconnect();
                    Serial.println(event);
                    
                    rEvt();
                }, SYSTEM_EVENT_STA_DISCONNECTED);
        };

        void connect(const char* ssid, const char* pass){
            Serial.print(ssid); Serial.print("["); Serial.print(strlen(ssid)); Serial.println("]");
            Serial.print(pass); Serial.print("["); Serial.print(strlen(pass)); Serial.println("]");
            WiFi.disconnect(true, true);
            WiFi.begin(ssid, pass);
        };
    private:
        OnConnectEvent cEvt;
        OnRejectEvent rEvt;
    };
#endif