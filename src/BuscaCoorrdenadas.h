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
    public:
        BuscaCoorrdenadas(Carrito *carrito1, Carrito *carrito2);
        ~BuscaCoorrdenadas();
        void irHasta(long pos1, long pos2);
        bool andar();
    };
    
    BuscaCoorrdenadas::BuscaCoorrdenadas(Carrito *carrito1, Carrito *carrito2)
    {
        _carrito1 = carrito1;
        _carrito2 = carrito2;
    }
    
    BuscaCoorrdenadas::~BuscaCoorrdenadas()
    {
    }

    void BuscaCoorrdenadas::irHasta(long pos1, long pos2)
    {
        auto distancia1 = abs(_carrito1->getPos() - pos1);
        auto distancia2 = abs(_carrito2->getPos() - pos2);

        auto distanciaMax = max(distancia1, distancia2);

        _carrito1->moveTo(pos1, 1000 * distancia1 / distanciaMax);
        _carrito2->moveTo(pos2, 1000 * distancia2 / distanciaMax);
    }

    bool BuscaCoorrdenadas::andar(){
        auto llego1 = _carrito1->andar();
        auto llego2 = _carrito2->andar();
        return llego1 && llego2;
    }

#endif