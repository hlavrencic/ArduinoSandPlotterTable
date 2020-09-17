#ifndef H_RedirectIndexHandler
    #define H_RedirectIndexHandler

    #include <ESPAsyncWebServer.h>
    

    class RedirectIndexHandler : public AsyncWebHandler {
    public:
        RedirectIndexHandler() {}
        virtual ~RedirectIndexHandler() {}

        bool canHandle(AsyncWebServerRequest *request){
            if(request->method() != HTTP_GET) return false; // solo captura los HttpGet
            
            auto url = request->url();
            auto esIndex = url.compareTo("/") == 0;
            return esIndex;
        }

        void handleRequest(AsyncWebServerRequest *request) {
            request->redirect("/index.html");
        }
    };
#endif