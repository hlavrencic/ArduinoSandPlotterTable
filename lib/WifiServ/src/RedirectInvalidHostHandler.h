#ifndef H_RedirectInvalidHostHandler
    #define H_RedirectInvalidHostHandler

    #include <ESPAsyncWebServer.h> 
    #include <wifiServ.h>   
    
    class RedirectInvalidHostHandler : public AsyncWebHandler {
    public:
        RedirectInvalidHostHandler(WifiServ* wifiServ) {
            _wifiServ = wifiServ;
        }
        virtual ~RedirectInvalidHostHandler() {}

        bool canHandle(AsyncWebServerRequest *request){   
            auto header = request->getHeader("Host");
            if(!header){
                return true;
            }

            auto hostValue = header->value();
            auto ip = _wifiServ->GetIP().toString();
            auto hostValido = hostValue.compareTo("connectivitycheck.gstatic.com")==0 || hostValue.compareTo(ip)==0;
            return !hostValido;
        }

        void handleRequest(AsyncWebServerRequest *request) {
            if(request->method() == HTTP_GET) {
                request->redirect("http://connectivitycheck.gstatic.com/generate_204");
            } else {
                request->send(404);
            }    
        }
    private:
        WifiServ* _wifiServ;
    };

#endif