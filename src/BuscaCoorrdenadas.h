#ifndef H_BuscaCoorrdenadas
    #define H_BuscaCoorrdenadas

    #include <Arduino.h>
    #include <Carrito.h>

    class BuscaCoorrdenadas
    {
    private:
        Carrito *_carrito1;
        Carrito *_carrito2;
        float _ajusteVelocidad1 = 0.4F;
        unsigned short VELOCIDAD_MAX = 1000;
        long _irHasta1 = 0;
        long _irHasta2 = 0;
    public:
        BuscaCoorrdenadas(Carrito *carrito1, Carrito *carrito2);
        ~BuscaCoorrdenadas();
        void irHasta(long pos1, long pos2);
        bool andar();
        long irHasta1();
        long irHasta2();
    };
    
    BuscaCoorrdenadas::BuscaCoorrdenadas(Carrito *carrito1, Carrito *carrito2)
    {
        _carrito1 = carrito1;
        _carrito2 = carrito2;
    }
    
    BuscaCoorrdenadas::~BuscaCoorrdenadas()
    {
    }

    long BuscaCoorrdenadas::irHasta1(){
        return _irHasta1;
    }

    long BuscaCoorrdenadas::irHasta2(){
        return _irHasta2;
    }

    void BuscaCoorrdenadas::irHasta(long pos1, long pos2)
    {
        _irHasta1 = pos1;
        _irHasta2 = pos2;

        auto distancia1 = abs(_carrito1->getPos() - pos1);
        auto distancia2 = abs(_carrito2->getPos() - pos2);

        float velocidad1 = distancia2 > 0 ? (distancia1 * _carrito2->getMaxSpeed()) / distancia2 : _carrito1->getMaxSpeed();
        float velocidad2 = distancia1 > 0 ? (distancia2 * _carrito1->getMaxSpeed()) / distancia1 : _carrito2->getMaxSpeed();
        if(velocidad1 > _carrito1->getMaxSpeed()){
            velocidad1 = _carrito1->getMaxSpeed();
        } else {
            velocidad2 = _carrito2->getMaxSpeed();
        }

        _carrito1->moveTo(pos1, velocidad1);
        _carrito2->moveTo(pos2, velocidad2);

    }

    bool BuscaCoorrdenadas::andar(){
        auto llego1 = _carrito1->andar();
        auto llego2 = _carrito2->andar();
        return llego1 && llego2;
    }

#endif