#include <CarritoUbicado.h>

CarritoUbicado::CarritoUbicado(Carrito *carrito){
    _carrito = carrito;
}

void CarritoUbicado::setPasosPorPixel(unsigned short pasosPorPixel){
    _pasosPorPixel = pasosPorPixel;
}

void CarritoUbicado::moverA(float speed, long pixelAbsoluto){

    _posAbsoluta = pixelAbsoluto * _pasosPorPixel ;
   
    auto sentido = 1;
    if(_carrito->pos() > _posAbsoluta) sentido = -1;

    _carrito->velocidad(speed * sentido);
}

long CarritoUbicado::pos(){
    return _carrito->pos();
}

bool CarritoUbicado::andar()
{
    auto llego = false;
    if(_carrito->pos() == _posAbsoluta) {
        _carrito->velocidad(0);
        llego = true;
    }
    
    auto limite = _carrito->andar();

    return llego || limite;
}