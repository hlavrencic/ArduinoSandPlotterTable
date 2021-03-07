#ifndef H_MarcoPolo
    #define H_MarcoPolo
    #include <Arduino.h>
    #include "BluetoothSerial.h"
    #include "WifiConnect.h"

    class MarcoPolo 
    {
        public:
            MarcoPolo(String btName);

            void loop(){
                read();
            };
        private:
            // Bluetooth Serial object
            BluetoothSerial SerialBT;
            WifiConnect* wifiConnect;

            void callback(String msg){
                String ssid, pass;
                
                bool separador = false;
                for(uint8_t i = 0; i < msg.length() && i < 100; i++){
                    char caracter = msg.charAt(i);
                    if(separador){
                        pass += caracter;
                    } else {
                        if(caracter == '@'){
                            separador = true;
                        } else {
                            ssid += caracter;
                        }
                    }
                }
                
                pass.trim();
                ssid.trim();

                wifiConnect->connect(ssid.c_str(), pass.c_str());
            };

            void read(){
                if (SerialBT.available()){
                    auto msg = SerialBT.readString();

                    Serial.println(msg);
                    callback(msg);
                }
            };

        
    };

    MarcoPolo::MarcoPolo(String btName)
    {
        if(!SerialBT.begin(btName)){
            Serial.println("Failed initializing BT");
            while(true){
                delay(10);
            }
        }

        wifiConnect = new WifiConnect([&](IPAddress ip){
            Serial.println("CONNECTED");
            if(!SerialBT.hasClient()) return;

            SerialBT.print("URL:");
            SerialBT.println(ip);
            SerialBT.flush();
        }, [&](){
            Serial.println("REJECTED");
            if(!SerialBT.hasClient()) return;

            SerialBT.println("REJECTED");
            SerialBT.flush();
        });
    };
#endif