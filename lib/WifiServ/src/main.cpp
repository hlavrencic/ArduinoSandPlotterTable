#include <WifiServ.h>

const uint8_t LED_PIN = 2;
WifiServ wifiServ;

void setup(){
    Serial.begin(230400);

    pinMode(LED_PIN, OUTPUT); // LED PIN;

    wifiServ.connectAP("My Access Point");

    wifiServ.textReceivedHandler = [&](StaticJsonDocument<200> doc){
        const char* pin2 = doc["pin2"];

        Serial.println(pin2);

        auto setLedTo = strcmp(pin2, "on") == 0;

        Serial.print("LED set to: ");
        Serial.println(setLedTo);

        digitalWrite(LED_PIN, setLedTo);
    };
}

void loop(){
    wifiServ.loop();
}