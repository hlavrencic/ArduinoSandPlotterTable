#include <Arduino.h>
#include <MarcoPolo.h>

MarcoPolo* marcoPolo;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(230400);

  marcoPolo = new MarcoPolo("ESP32");
}

void loop() {
  // put your main code here, to run repeatedly:
  marcoPolo->loop();
}