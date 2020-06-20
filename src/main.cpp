#include <Arduino.h>
#include <AccelStepper.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <CarritoUbicado.h>
#include <BuscaCoorrdenadas.h>



const byte dirPin = 8;
const byte stepPin = 9;
const byte motorPin1 = 3;      // IN1 on the ULN2003 driver
const byte motorPin2 = 4;      // IN2 on the ULN2003 driver
const byte motorPin3 = 5;     // IN3 on the ULN2003 driver
const byte motorPin4 = 6;     // IN4 on the ULN2003 driver


AccelStepper stepper = AccelStepper(1, stepPin, dirPin);
Carrito carrito1 = Carrito(&stepper, PIN7, 3000U, true);
CarritoUbicado carritoUbicado1 = CarritoUbicado(&carrito1);


AccelStepper stepper2 = AccelStepper(8, motorPin1, motorPin3, motorPin2, motorPin4);
Carrito carrito2 = Carrito(&stepper2, 0, 30000U, true);
CarritoUbicado carritoUbicado2 = CarritoUbicado(&carrito2);

BuscaCoorrdenadas buscaCoorrdenadas = BuscaCoorrdenadas(&carritoUbicado1, &carritoUbicado2);

LiquidCrystal_I2C lcd(0x3F,16,2);
uint8_t bell[8]  = {0x4,0xe,0xe,0xe,0x1f,0x0,0x4};
uint8_t note[8]  = {0x2,0x3,0x2,0xe,0x1e,0xc,0x0};
uint8_t clock[8] = {0x0,0xe,0x15,0x17,0x11,0xe,0x0};
uint8_t heart[8] = {0x0,0xa,0x1f,0x1f,0xe,0x4,0x0};
uint8_t duck[8]  = {0x0,0xc,0x1d,0xf,0xf,0x6,0x0};
uint8_t check[8] = {0x0,0x1,0x3,0x16,0x1c,0x8,0x0};
uint8_t cross[8] = {0x0,0x1b,0xe,0x4,0xe,0x1b,0x0};
uint8_t retarrow[8] = {	0x1,0x1,0x5,0x9,0x1f,0x8,0x4};

enum Mode { manual, autoPos, calibrationMode, calibrationMode2 };
Mode mode = manual;

enum MenuOption {noMenu, menu, opcion1};
MenuOption menuOption = noMenu;

void btnSwitch(){
   
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
   
   // Marcar los pines como salida

   pinMode(LED_BUILTIN, OUTPUT);

   pinMode(A2, INPUT);
   pinMode(A3, INPUT);
   pinMode(PIN2, INPUT_PULLUP);

   pinMode(PIN3, OUTPUT);
   pinMode(PIN4, OUTPUT);
   pinMode(PIN5, OUTPUT);
   pinMode(PIN6, OUTPUT);

   pinMode(PIN7, OUTPUT);
   pinMode(dirPin, OUTPUT);
   pinMode(stepPin, OUTPUT);

   pinMode(10, INPUT_PULLUP);
   pinMode(11, INPUT);

   Serial.begin(230400);

   carritoUbicado1.setPasosPorPixel(1);
   carritoUbicado2.setPasosPorPixel(5);

   lcd.init();                      // initialize the lcd 
   lcd.backlight();

   lcd.createChar(0, bell);
   lcd.createChar(1, note);
   lcd.createChar(2, clock);
   lcd.createChar(3, heart);
   lcd.createChar(4, duck);
   lcd.createChar(5, check);
   lcd.createChar(6, cross);
   lcd.createChar(7, retarrow);
   lcd.home();
   lcd.print("Calibrando...");
   mode = calibrationMode;
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

bool moving = false;
bool btnOn = false;

byte posStep = 0;
void autoPosMove(){
   bool llego = false;
   
   if(posStep == 0) {
      llego = true;
   } else {
      llego = buscaCoorrdenadas.andar();
   }
   
   if(llego){
      posStep++;
      switch (posStep)
      {
      case 1:
         buscaCoorrdenadas.irHasta(1000L, 2000L);
         break;
      case 2:
         buscaCoorrdenadas.irHasta(3000L, 500L);
         break;
      default:
         posStep = 0;
         buscaCoorrdenadas.irHasta(2000L, 1000L);
      }      

      
   }

}

void analogManualMove(){
   auto a2 = analogRead(A2);
   auto a3 = analogRead(A3);

   auto logA2 = logSpeed(a2);
   auto logA3 = logSpeed(a3);

   stepper.setSpeed(logA2/6);
   stepper2.setSpeed(logA3);
   carrito1.andar();
   carrito2.andar();

   if(logA3 == 0){
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

   auto calibrado1 = carrito1.calibrar(digitalRead(11));
   auto calibrado2 = carrito2.calibrar(!digitalRead(10));
   
   

   if(!calibrado1){
      return;
   }

   if(!calibrado2){
      return;
   }

   mode = calibrationMode2;
   lcd.clear();
   lcd.print("Calibracion 2...");
}

void calibrate2(){
   auto calibrado = buscaCoorrdenadas.calibrar();

   if(!calibrado) return;

   mode = manual;
   lcd.clear();
   lcd.print("Listo.");
}

void loop() {
   auto d2 = digitalRead(PIN2);

   if(!d2){
      if(btnOn) return;
      btnOn=true;
      mode = manual;
      btnSwitch();
      return;
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

      case calibrationMode2:
         calibrate2();
         break;
      default:
         break;
   }
}