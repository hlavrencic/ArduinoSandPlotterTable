#ifndef H_Carrito
    #define H_Carrito

    #include <AccelStepper.h>

    class Carrito {
        public:
            Carrito(AccelStepper *stepper, byte disablePin, unsigned int maxPos, bool sensorBajando);
            void velocidad(float speed);
            bool andar();
            bool calibrar(bool choqueConSensor);
            bool estoyEnElLimite();
            long pos();
        private:
            void _establecerSentido(int8_t sentido);
            AccelStepper *_stepper;
            byte _disablePin;
            unsigned int _maxPos;
            int8_t _horientacion;
            int8_t _sentido;
            bool _calibrando = false;
    };

#endif