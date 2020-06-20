#ifndef H_CarritoUbicado
    #define H_CarritoUbicado

    #include <Carrito.h>

    class CarritoUbicado {
        public:
            CarritoUbicado(Carrito *carrito);
            void moverA(float speed, long posAbsoluta);
            void setPasosPorPixel(unsigned short pasosPorPixel);
            long pos();
            bool andar();
        private:
            Carrito *_carrito;
            unsigned short _pasosPorPixel = 1;
            long _posAbsoluta = 0;
    };

#endif