#ifndef H_Carrito
    #define H_Carrito

    #include <AccelStepper.h>

    class Carrito {
        public:
            Carrito(AccelStepper *stepper, byte disablePin, unsigned int maxPos, bool sensorBajando, bool logOn);
            bool andar();
            bool calibrar(bool choqueConSensor);
            void setSpeed(float speed);
            void moveTo(long pos, float speed);
            long getPos();
        private:
            AccelStepper *_stepper;
            byte _disablePin;
            unsigned int _maxPos;
            float _horientacion;
            int8_t _sentido;
            bool _calibrando = false;
            unsigned int cont = 0;
            bool _logOn;
            bool _error;
    };

    Carrito::Carrito(
        AccelStepper *stepper, 
        byte disablePin,
        unsigned int maxPos,
        bool sensorBajando,
        bool logOn
        ){
        _stepper = stepper;
        _disablePin = disablePin;
        _maxPos = maxPos;
        _logOn = logOn;

        if(sensorBajando) _horientacion = 1;
        else _horientacion = -1;

        _stepper->setAcceleration(1000);
        _stepper->setMaxSpeed(1000);
    }

    void Carrito::setSpeed(float speed){
        speed = speed * _horientacion;
        _stepper->setSpeed(speed);

        int8_t sentidoActual;
        if(speed < 0) sentidoActual = -1;
        else if (speed > 0) sentidoActual = 1;
        else sentidoActual = 0;

        if(_sentido == sentidoActual){
            return;
        }

        _sentido = sentidoActual;
            
        if(_sentido != 0){
            // Encender
            _stepper->enableOutputs();
            if(_disablePin) digitalWrite(_disablePin, LOW);
            
        } else {
            _stepper->disableOutputs();
            if(_disablePin) digitalWrite(_disablePin, HIGH);
        }        
    }

    bool Carrito::calibrar(bool choqueConSensor){
        if(!_calibrando && !choqueConSensor){
            _calibrando = true;
            _stepper->setCurrentPosition(_maxPos);// Asumo que estoy en el otro extremo y comienzo a bajar
            setSpeed(-1000);
        }

        if(choqueConSensor){
            _calibrando = false;
            _stepper->setCurrentPosition(0);
            setSpeed(0);
            return true;
        }

        auto fin = andar();
        if(fin && !_error){
            _error = true;
            Serial.println("FIN");
        }

        return false;
    }

    bool Carrito::andar(){
        if(_stepper->speed() == 0) return true;

        auto pos = _stepper->currentPosition();

        int8_t estoyEnElLimite = 0;
        if(pos <= 0){
            estoyEnElLimite = -1;
        } else if(pos >= _maxPos){
            estoyEnElLimite = 1;
        } else if (pos == _stepper->targetPosition()){
            setSpeed(0);
            return true;
        }

        if(estoyEnElLimite * _sentido == 1 ){ // Sin choque
            setSpeed(0);
            return true;
        } 

        _stepper->runSpeed();

        return false;
    }

    void Carrito::moveTo(long pos, float speed){
        _stepper->moveTo(pos);

        if(_stepper->currentPosition() > pos) speed = -speed;  // Invierto el sentido
        setSpeed(speed);
    }

    long Carrito::getPos(){
        return _stepper->currentPosition();
    }

#endif