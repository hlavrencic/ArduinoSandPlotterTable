#ifndef FSInclude

    #ifdef ARDUINO_ARCH_ESP32
        #include <SPIFFS.h>
        #define FSInclude SPIFFS
    #else
        #include <FS.h>
        #define FSInclude SPIFFS
    #endif

    
#endif