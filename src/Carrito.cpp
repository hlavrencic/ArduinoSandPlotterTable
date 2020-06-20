#include <Carrito.h>

Carrito::Carrito(
    AccelStepper *stepper, 
    byte disablePin,
    unsigned int maxPos,
    bool sensorBajando
    ){
    _stepper = stepper;
    _disablePin = disablePin;
    _maxPos = maxPos;
    
    if(sensorBajando) _horientacion = 1;
    else _horientacion = -1;

    _stepper->setMaxSpeed(1000);
}

bool Carrito::calibrar(bool choqueConSensor){
    if(!_calibrando){
        _calibrando = true;
        _stepper->setCurrentPosition(_maxPos); // Asumo que estoy en el otro extremo y comienzo a bajar
        _stepper->setSpeed(-1000 * _horientacion);
    }

    if(choqueConSensor){
        _calibrando = false;
        _stepper->setCurrentPosition(0);
        return true;
    }

    auto fin = andar();
    if(fin){
        Serial.println("FIN");
    }

    return false;
}

long Carrito::pos(){
    return _stepper->currentPosition();
}

void Carrito::velocidad(float valor){
    _stepper->setSpeed(valor);
}

bool Carrito::andar(){
    auto pos = _stepper->currentPosition();

    int8_t estoyEnElLimite = 0;
    if(pos <= 0){
        estoyEnElLimite = -1;
    } else if(pos >= _maxPos){
        estoyEnElLimite = 1;
    }

    auto speed = _stepper->speed();

    int8_t sentidoActual;
    if(speed < 0) sentidoActual = -1;
    else if (speed > 0) sentidoActual = 1;
    else sentidoActual = 0;

    if(estoyEnElLimite * sentidoActual == 1 ){ // Sin choque
        _stepper->setSpeed(0);
        _establecerSentido(0);
        return true;
    } 

    _stepper->setSpeed(speed);

    _establecerSentido(sentidoActual);
    
    _stepper->runSpeed();

    return false;
}

void Carrito::_establecerSentido(int8_t sentido){
    if(_sentido == sentido){
        return;
    }

    _sentido = sentido;
    
    if(sentido != 0){
        // Encender
        _stepper->enableOutputs();
        if(_disablePin) digitalWrite(_disablePin, HIGH);
        
    } else {
        _stepper->disableOutputs();
        if(_disablePin) digitalWrite(_disablePin, LOW);
    }
}