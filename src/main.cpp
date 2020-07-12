#include <Arduino.h>
#include <AccelStepper.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <BuscaCoorrdenadas.h>

#ifdef ARDUINO_ARCH_ESP32
//include ESP32 specific libs
   const byte dirPin = GPIO_NUM_16;
   const byte stepPin = GPIO_NUM_4;
   const byte motorPin1 = GPIO_NUM_23;      // IN1 on the ULN2003 driver
   const byte motorPin2 = GPIO_NUM_19;      // IN2 on the ULN2003 driver
   const byte motorPin3 = GPIO_NUM_18;     // IN3 on the ULN2003 driver
   const byte motorPin4 = GPIO_NUM_17;     // IN4 on the ULN2003 driver
   const byte motor1EnabledPin = GPIO_NUM_5;     // IN4 on the ULN2003 driver

   const byte analog1Pin = GPIO_NUM_34;     // Analog
   const byte analog2Pin = GPIO_NUM_35;     // Analog
   const byte analogSwitchPin = GPIO_NUM_32;     // Analog

   const byte end1Pin = GPIO_NUM_33;
   const byte end2Pin = GPIO_NUM_27  ;
#else  
   

   const byte dirPin = 8;
   const byte stepPin = 9;
   const byte motorPin1 = 3;      // IN1 on the ULN2003 driver
   const byte motorPin2 = 4;      // IN2 on the ULN2003 driver
   const byte motorPin3 = 5;     // IN3 on the ULN2003 driver
   const byte motorPin4 = 6;     // IN4 on the ULN2003 driver
   const byte motor1EnabledPin = 7;     // IN4 on the ULN2003 driver

   const byte analog1Pin = A2;     // Analog
   const byte analog2Pin = A3;     // Analog
   const byte analogSwitchPin = 2;     // Analog

   const byte end1Pin = 10;
   const byte end2Pin = 11;
#endif


AccelStepper stepper = AccelStepper(1, stepPin, dirPin);
Carrito carrito1 = Carrito(&stepper, motor1EnabledPin, 6000U, true, false);

AccelStepper stepper2 = AccelStepper(8, motorPin1, motorPin3, motorPin2, motorPin4);
Carrito carrito2 = Carrito(&stepper2, 0, 35000U, true, false);

BuscaCoorrdenadas buscaCoorrdenadas = BuscaCoorrdenadas(&carrito1, &carrito2);

LiquidCrystal_I2C lcd(0x3F, 16, 2);
uint8_t bell[8]  = {0x4,0xe,0xe,0xe,0x1f,0x0,0x4};
uint8_t note[8]  = {0x2,0x3,0x2,0xe,0x1e,0xc,0x0};
uint8_t heart[8] = {0x0,0xa,0x1f,0x1f,0xe,0x4,0x0};
uint8_t duck[8]  = {0x0,0xc,0x1d,0xf,0xf,0x6,0x0};
uint8_t check[8] = {0x0,0x1,0x3,0x16,0x1c,0x8,0x0};
uint8_t cross[8] = {0x0,0x1b,0xe,0x4,0xe,0x1b,0x0};
uint8_t retarrow[8] = {	0x1,0x1,0x5,0x9,0x1f,0x8,0x4};

enum Mode { manual, autoPos, calibrationMode };
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

void setup() {
   Serial.begin(230400);
   // Marcar los pines como salida
   Serial.println("Setup PINS...");

   pinMode(LED_BUILTIN, OUTPUT);

   pinMode(analog1Pin, INPUT);
   pinMode(analog2Pin, INPUT);
   pinMode(analogSwitchPin, INPUT_PULLUP);

   pinMode(motorPin1, OUTPUT);
   pinMode(motorPin2, OUTPUT);
   pinMode(motorPin3, OUTPUT);
   pinMode(motorPin4, OUTPUT);

   pinMode(motor1EnabledPin, OUTPUT);
   pinMode(dirPin, OUTPUT);
   pinMode(stepPin, OUTPUT);

   pinMode(end1Pin, INPUT_PULLUP);
   pinMode(end2Pin, INPUT_PULLUP);

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
      auto pos2 = carrito2.getPos();
      if(pos2 > 30000) forward = false; else if(pos2 <= 0) forward = true;
      if(forward) pos2 += INTERVALO; else pos2 -= INTERVALO;
      
      auto cos2 = (double)pos2;
      cos2 = cos(cos2/5000);
      auto pos1 = (cos2 * 3000) + 3000;
      buscaCoorrdenadas.irHasta(pos1, pos2);
   }
}

void analogManualMove(){
   auto a2 = analogRead(analog1Pin);
   auto a3 = analogRead(analog2Pin);

   auto logA2 = logSpeed(a2);
   auto logA3 = logSpeed(a3);

   print(logA2);

   carrito1.setSpeed(logA2/6);
   carrito2.setSpeed(logA3);
   carrito1.andar();
   carrito2.andar();

   if(logA3 == 0 && logA2 == 0){
      if(moving){
         lcd.clear();
         lcd.print("Detenido en ");
         lcd.setCursor(0,1);
         lcd.print(stepper.currentPosition());
         lcd.print(" - ");
         lcd.print(stepper2.currentPosition());
      }
      moving = false;
   } else {
      menuOption = noMenu;
      if(!moving){
         lcd.clear();
         lcd.print("Moviendo el bote");
      }

      moving = true;
   }
}

void calibrate(){

   auto calibrado1 = carrito1.calibrar(!digitalRead(end1Pin));
   auto calibrado2 = carrito2.calibrar(!digitalRead(end2Pin));

   if(!calibrado1){
      return;
   }

   if(!calibrado2){
      return;
   }

   Serial.println("Calibrado.");

   mode = Mode::manual;
   lcd.clear();
   lcd.print("Listo.");
}

void loop() {
   auto d2 = digitalRead(analogSwitchPin);

   if(!d2){
      if(btnOn) return;
      btnOn=true;

      Serial.println("Button pressed");
      
      btnSwitch();
   } else {
      btnOn=false;
   }

   switch (mode)
   {
      case manual:
         analogManualMove();
         break;
      
      case autoPos:
         autoPosMove();
         break;

      case calibrationMode:
         calibrate();
         break;

      default:
         break;
   }
}