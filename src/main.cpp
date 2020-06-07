#include <Arduino.h>
#include <AccelStepper.h>

const byte dirPin = 8;
const byte stepPin = 9;
const byte motorPin1 = 3;      // IN1 on the ULN2003 driver
const byte motorPin2 = 4;      // IN2 on the ULN2003 driver
const byte motorPin3 = 5;     // IN3 on the ULN2003 driver
const byte motorPin4 = 6;     // IN4 on the ULN2003 driver

AccelStepper stepper = AccelStepper(1, stepPin, dirPin);
AccelStepper stepper2 = AccelStepper(8, motorPin1, motorPin3, motorPin2, motorPin4);

void setup() {
   
   // Marcar los pines como salida
   pinMode(dirPin, OUTPUT);
   pinMode(stepPin, OUTPUT);
   pinMode(LED_BUILTIN, OUTPUT);

   pinMode(A2, INPUT);
   pinMode(A3, INPUT);

   pinMode(PIN3, OUTPUT);
   pinMode(PIN4, OUTPUT);
   pinMode(PIN5, OUTPUT);
   pinMode(PIN6, OUTPUT);

   Serial.begin(230400);

   // Set the maximum speed in steps per second:
   stepper.setMaxSpeed(1000);
   stepper2.setMaxSpeed(1000);
}
 
unsigned long lastMicros = 0;
unsigned long lastPrint = 0;

float logSpeed(int a2){

   auto direction = false;

   if(a2 <= 400){
      if(a2 < 0) a2 = 0;
      a2 = 400 - a2;
      direction = false;
   } else if(a2 >= 600 ){
      direction = true;
      a2 -= 600;
   } else {
      a2 = 0;
   }

   

   if(a2 >= 400){
      a2 = 399;
   }

   auto logA2 = pow(a2, 2) / ( pow(400, 2) / 1023);

   if(direction){
      logA2 = logA2 * (-1);
   }

   return logA2;
}

void loop() {
   auto m = micros();
   auto a2 = analogRead(A2);
   auto a3 = analogRead(A3);

   auto logA2 = logSpeed(a2);
   auto logA3 = logSpeed(a3);


   if(m - lastPrint > 500000 && (abs(logA2) >= 1 || abs(logA3) >= 1)){
      Serial.print(a2);
      Serial.print("/");
      Serial.print(a3);
      Serial.print(" ");
      Serial.print(logA2);
      Serial.print("/");
      Serial.println(logA3);
      lastPrint = m;
   }



   stepper.setSpeed(logA2/4);
   stepper.runSpeed();

   
   if(logA3 == 0){
      stepper2.disableOutputs();
   } else {
      stepper2.enableOutputs();
   }
   stepper2.setSpeed(logA3);
   stepper2.runSpeed();

}