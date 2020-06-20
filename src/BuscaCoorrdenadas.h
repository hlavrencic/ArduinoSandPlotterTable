#ifndef H_BuscaCoorrdenadas
    #define H_BuscaCoorrdenadas

    #include <Arduino.h>
    #include <CarritoUbicado.h>

    class BuscaCoorrdenadas
    {
    private:
        CarritoUbicado *_carritoUbicado1;
        CarritoUbicado *_carritoUbicado2;
        float _ajusteVelocidad1 = 0.4F;
        byte _calibrando = 0;
        unsigned short VELOCIDAD_MAX = 1000;
    public:
        BuscaCoorrdenadas(CarritoUbicado *carritoUbicado1, CarritoUbicado *carritoUbicado2);
        ~BuscaCoorrdenadas();
        void irHasta(long pos1, long pos2);
        bool andar();
        bool andar(float velocidad1, float velocidad2);
        bool calibrar();
    };
    
    BuscaCoorrdenadas::BuscaCoorrdenadas(CarritoUbicado *carritoUbicado1, CarritoUbicado *carritoUbicado2)
    {
        _carritoUbicado1 = carritoUbicado1;
        _carritoUbicado2 = carritoUbicado2;
    }
    
    BuscaCoorrdenadas::~BuscaCoorrdenadas()
    {
    }

    void BuscaCoorrdenadas::irHasta(long pos1, long pos2)
    {
        auto resto1 = abs(pos1 - _carritoUbicado1->pos()) ;
        auto resto2 = abs(pos2 - _carritoUbicado2->pos());

        auto restoMax = max(resto1, resto2);
        
        _carritoUbicado1->moverA(_ajusteVelocidad1 * VELOCIDAD_MAX * resto1/restoMax, pos1);
        _carritoUbicado2->moverA(VELOCIDAD_MAX * resto2/restoMax, pos2);
    }

    bool BuscaCoorrdenadas::andar(){
        auto llego1 = _carritoUbicado1->andar();
        auto llego2 = _carritoUbicado2->andar();
        return llego1 && llego2;
    }

    bool BuscaCoorrdenadas::calibrar(){

        switch (_calibrando)
        {
        case 0: 
            _carritoUbicado1->moverA(VELOCIDAD_MAX, 2000);
            _carritoUbicado2->moverA(VELOCIDAD_MAX, 2000);
            _calibrando++;
            break;
        case 1:
            {
                auto llego1 = _carritoUbicado1->andar();
                auto llego2 = _carritoUbicado2->andar();
                if(llego1 && llego2){
                    _calibrando++;
                }
            }
            break;
            
        case 2:
            _carritoUbicado1->moverA(VELOCIDAD_MAX, 1000);
            _carritoUbicado2->moverA(VELOCIDAD_MAX, 1000);
            _calibrando++;
            break;
        case 3:
            {
                auto llego1 = _carritoUbicado1->andar();
                auto llego2 = _carritoUbicado2->andar();
                if(llego1 || llego2){
                    _calibrando = 0;
                    auto pos1 = _carritoUbicado1->pos();
                    auto pos2 = _carritoUbicado2->pos();
                    _ajusteVelocidad1 = pos2/pos1;

                    Serial.print("Calibracion: ");
                    Serial.print(pos2);
                    Serial.print("/");
                    Serial.print(pos1);
                    Serial.print("   Ajuste: ");
                    Serial.println(_ajusteVelocidad1);

                    return true;
                }
            }
            break;
        }

        return false;
    }

    bool BuscaCoorrdenadas::andar(float velocidad1, float velocidad2){

        long pos1 = 0;
        if(velocidad1 > 0) pos1 = 999999;
        _carritoUbicado1->moverA(velocidad1, 2000);
    }

#endif