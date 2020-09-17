#include <Arduino.h>
#include <AccelStepper.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <BuscaCoorrdenadas.h>
#include <WifiServ.h>

WifiServ wifiServ;

#ifdef ARDUINO_ARCH_ESP32
//include ESP32 specific libs
   const byte motor1Pin1 = GPIO_NUM_2;  
   const byte motor1Pin2 = GPIO_NUM_4;  
   const byte motor1Pin3 = GPIO_NUM_16; 
   const byte motor1Pin4 = GPIO_NUM_17; 

   const byte motor2Pin1 = GPIO_NUM_23;    
   const byte motor2Pin2 = GPIO_NUM_19;    
   const byte motor2Pin3 = GPIO_NUM_18;    
   const byte motor2Pin4 = GPIO_NUM_5;    

   const byte analog1Pin = GPIO_NUM_34;     // Analog
   const byte analog2Pin = GPIO_NUM_35;     // Analog
   const byte analogSwitchPin = GPIO_NUM_32;     // Analog

   const byte end1Pin = GPIO_NUM_33;
   const byte end2Pin = GPIO_NUM_27  ;
#else  
   

   const byte dirPin = 8;
   const byte stepPin = 9;
   const byte motor2Pin1 = 3;      // IN1 on the ULN2003 driver
   const byte motor2Pin2 = 4;      // IN2 on the ULN2003 driver
   const byte motor2Pin3 = 5;     // IN3 on the ULN2003 driver
   const byte motor2Pin4 = 6;     // IN4 on the ULN2003 driver
   const byte motor1EnabledPin = 7;     // IN4 on the ULN2003 driver

   const byte analog1Pin = A2;     // Analog
   const byte analog2Pin = A3;     // Analog
   const byte analogSwitchPin = 2;     // Analog

   const byte end1Pin = 10;
   const byte end2Pin = 11;
#endif

unsigned long contadorInactividad = 0;

AccelStepper stepper = AccelStepper(8, motor1Pin3, motor1Pin1 , motor1Pin2, motor1Pin4);
Carrito carrito1 = Carrito(&stepper, 4000U, false, end1Pin);

AccelStepper stepper2 = AccelStepper(8, motor2Pin4, motor2Pin2, motor2Pin1, motor2Pin3);
Carrito carrito2 = Carrito(&stepper2, 35000U, false, end2Pin);

BuscaCoorrdenadas buscaCoorrdenadas = BuscaCoorrdenadas(&carrito1, &carrito2);

LiquidCrystal_I2C lcd(0x3F, 16, 2);
uint8_t bell[8]  = {0x4,0xe,0xe,0xe,0x1f,0x0,0x4};
uint8_t note[8]  = {0x2,0x3,0x2,0xe,0x1e,0xc,0x0};
uint8_t heart[8] = {0x0,0xa,0x1f,0x1f,0xe,0x4,0x0};
uint8_t duck[8]  = {0x0,0xc,0x1d,0xf,0xf,0x6,0x0};
uint8_t check[8] = {0x0,0x1,0x3,0x16,0x1c,0x8,0x0};
uint8_t cross[8] = {0x0,0x1b,0xe,0x4,0xe,0x1b,0x0};
uint8_t retarrow[8] = {	0x1,0x1,0x5,0x9,0x1f,0x8,0x4};

char wsMsg[1000];

enum Mode { manual, autoPos, calibrationMode, irA };
Mode mode = manual;

enum MenuOption {noMenu, menu, opcion1};
MenuOption menuOption = noMenu;

unsigned long lastMillis;
void print(char * txt){
   auto m = millis();
   if(m - lastMillis > 500){
      lastMillis = m;
      Serial.println(txt);
   }
}

void print(double txt){
   auto m = millis();
   if(m - lastMillis > 500){
      lastMillis = m;
      Serial.println(txt);
   }
}

void btnSwitch(){
   if(mode == Mode::calibrationMode){
      return;
   }

   switch (menuOption)
   {
   case opcion1:
      mode = autoPos;
      menuOption = noMenu;
      lcd.clear();
      lcd.print("Me pongo locooo");
      break;

   case menu:
      menuOption = opcion1;
      break;
   
   default:
      menuOption = menu;
      mode = manual;
      lcd.clear();
      lcd.print("Te bailo ?");
      
      break;
   }
}

void sendSocketMsg(const char* msg){
   auto doc = wifiServ.initJson();
   doc["msg"] = msg;
   wifiServ.sendJson(doc);
}

void sendPos(){
   auto doc = wifiServ.initJson();
   doc["xPos"] = stepper.currentPosition();
   doc["yPos"] = stepper2.currentPosition();
   wifiServ.sendJson(doc);
}

void setup() {
   Serial.begin(230400);
   // Marcar los pines como salida
   Serial.println("Setup PINS...");

   pinMode(analog1Pin, INPUT);
   pinMode(analog2Pin, INPUT);
   pinMode(analogSwitchPin, INPUT_PULLUP);

   pinMode(motor1Pin1, OUTPUT);
   pinMode(motor1Pin2, OUTPUT);
   pinMode(motor1Pin3, OUTPUT);
   pinMode(motor1Pin4, OUTPUT);

   pinMode(motor2Pin1, OUTPUT);
   pinMode(motor2Pin2, OUTPUT);
   pinMode(motor2Pin3, OUTPUT);
   pinMode(motor2Pin4, OUTPUT);

   pinMode(end1Pin, INPUT_PULLUP);
   pinMode(end2Pin, INPUT_PULLUP);

   stepper.setMaxSpeed(300);
   stepper2.setMaxSpeed(1000);

   Serial.println("LCD init...");

   lcd.init();                      // initialize the lcd 
   lcd.backlight();

   lcd.createChar(0, bell);
   lcd.createChar(1, note);
   lcd.createChar(3, heart);
   lcd.createChar(4, duck);
   lcd.createChar(5, check);
   lcd.createChar(6, cross);
   lcd.createChar(7, retarrow);
   lcd.home();
   lcd.print("Calibrando...");
   mode = calibrationMode;

   wifiServ.connectAP("MesitaArena");
   wifiServ.connectedHandler = [&](){
      sendSocketMsg("Hello from ESP32 Server");
   };

   wifiServ.textReceivedHandler = [&](StaticJsonDocument<200> doc){
      if(mode == Mode::calibrationMode) return;
      
      if(doc.containsKey("ssid")){
         Serial.print("Nueva red: ");
         const char* ssid = doc["ssid"];
         const char* pass = doc["pass"];
         Serial.print(ssid);
         Serial.println(pass);

         wifiServ.connect(ssid, pass);
         return;
      }

      const char* xPos = doc["xPos"];

      if(strcmp(xPos, "AUTO") == 0){
         mode = Mode::autoPos;
         sendSocketMsg("Modo AUTO");
         return;
      } else if(strcmp(xPos, "MANUAL") == 0){
         mode = Mode::manual;
         sendSocketMsg("Modo MANUAL");
         return;
      } else if(strcmp(xPos, "CALIB") == 0){
         mode = Mode::calibrationMode;
         sendSocketMsg("Calibrando");
         return;
      }

      const char* yPos = doc["yPos"];
      char * xPosPointer;
      char * yPosPointer;
      auto pos1 = strtol(xPos, &xPosPointer, 10);
      auto pos2 = strtol(yPos, &yPosPointer, 10);
      buscaCoorrdenadas.irHasta(pos1, pos2);
      mode = Mode::irA;
      sendSocketMsg("moviendo A...");
   };
   Serial.println("End setup.");
}

float logSpeed(int x){
   const int CENTER = 2000;
   auto y = pow(x-CENTER, 3) / ( pow(CENTER,3) / 1000);
   if(abs(y) < 10) y = 0;
   return y;
}

bool moving = false;
bool btnOn = false;

bool forward = true;
void autoPosMove(){
   const long INTERVALO = 100;
   if(buscaCoorrdenadas.andar()){
      sendPos();

      auto pos2 = carrito2.getPos();
      if(pos2 >= 35000) forward = false; else if(pos2 <= 5000) forward = true;
      if(forward) pos2 += INTERVALO; else pos2 -= INTERVALO;
      
      auto cos2 = (double)pos2;
      cos2 = cos(cos2/6000);
      auto pos1 = (cos2 * 2000) + 2000;
      buscaCoorrdenadas.irHasta(pos1, pos2);
   }
}

void moverA(){
   if(buscaCoorrdenadas.andar()){
      mode = Mode::manual;
      //char ff[60];
      //sprintf(ff, "Llegue a %ld, %ld", buscaCoorrdenadas.irHasta1(), buscaCoorrdenadas.irHasta2());
      //sendSocketMsg(ff);
      sendPos();
   }
}



bool analogManualMove(){
   auto a2 = analogRead(analog1Pin);
   auto a3 = analogRead(analog2Pin);

   auto logA2 = logSpeed(a2);
   auto logA3 = logSpeed(a3);

   if(logA3 <= 1 && logA2 <= 1){
      if(moving){
         lcd.clear();
         lcd.print("Detenido en ");
         lcd.setCursor(0,1);
         lcd.print(stepper.currentPosition());
         lcd.print(" - ");
         lcd.print(stepper2.currentPosition());

         sendPos();
      }
      moving = false;

      return false;
   }

   carrito1.setSpeed(logA2/6);
   carrito2.setSpeed(logA3);
   carrito1.andar();
   carrito2.andar();
   menuOption = noMenu;
   
   if(!moving){
      lcd.clear();
      lcd.print("Moviendo el bote");
   }
   moving = true;

   return true;
}

void calibrate(){

   auto calibrado1 = carrito1.calibrar();
   auto calibrado2 = carrito2.calibrar();

   if(!calibrado1){
      return;
   }

   if(!calibrado2){
      return;
   }

   sendSocketMsg("Calibrado.");
   Serial.println("Calibrado.");
   mode = Mode::manual;
   lcd.clear();
   lcd.print("Modo Manual.");
}

void enableMotors(bool state){
   if(state){
      contadorInactividad = millis();
      stepper.enableOutputs();
      stepper2.enableOutputs();
   } else {
      stepper.disableOutputs();
      stepper2.disableOutputs();      
   }
}

void loop() {
   wifiServ.loop();
   auto d2 = digitalRead(analogSwitchPin);

   if(!d2){
      if(btnOn) return;
      btnOn=true;

      Serial.println("Button pressed");
      
      btnSwitch();
   } else {
      btnOn=false;
   }

   if(mode != Mode::manual){
      enableMotors(true);
   }

   switch (mode)
   {
      case Mode::manual:
         if(analogManualMove()) enableMotors(true);
         break;
      
      case Mode::autoPos:
         autoPosMove();
         break;

      case Mode::calibrationMode:
         calibrate();
         break;
      case Mode::irA:
         moverA();
         break;
      default:
         break;
   }

   if(millis() - contadorInactividad >= 100){
      enableMotors(false);
   }
}