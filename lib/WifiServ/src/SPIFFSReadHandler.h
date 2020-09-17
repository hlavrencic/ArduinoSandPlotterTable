#ifndef H_SPIFFSReadHandler
    #define H_SPIFFSReadHandler

    #include <ESPAsyncWebServer.h> 
    #include <FSInclude.h>

    class SPIFFSReadHandler : public AsyncWebHandler {
    public:
        SPIFFSReadHandler() {}
        virtual ~SPIFFSReadHandler() {}

        static void listDir(const char * dirname){
            Serial.printf("Listing directory: %s\r\n", dirname);

            File root = SPIFFS.open(dirname, "r");
            if(!root){
                Serial.println("- failed to open directory");
                return;
            }
            if(!root.isDirectory()){
                Serial.println(" - not a directory");
                return;
            }

            File file = root.openNextFile();
            while(file){
                if(file.isDirectory()){
                    Serial.print("  DIR : ");
                    Serial.println(file.name());
                    listDir(file.name());
                } else {
                    Serial.print("  FILE: ");
                    Serial.print(file.name());
                    Serial.print("\tSIZE: ");
                    Serial.println(file.size());
                }
                file = root.openNextFile();
            }
        };

        bool canHandle(AsyncWebServerRequest *request){
            if(request->method() != HTTP_GET) return false; // solo captura los HttpGet

            // Verifica que el archivo exista
            auto url = request->url().c_str();
            auto file = SPIFFS.open(url, "r");
            auto existe = file.available();
            file.close();
            return existe;
        }

        void handleRequest(AsyncWebServerRequest *request) {
            auto urlStr = request->url();
            auto contentType = _getType(urlStr);
            request->send(SPIFFS, urlStr, contentType);
        }
    private:
        bool _hasExt(String path, const char* ext){
            auto i = path.lastIndexOf(ext);
            auto len = (int)path.length();
            auto hasExt = i > 0 && i >= len - 5;
            return hasExt;
        }

        const char * _getType(String path){
            if(_hasExt(path, ".jpg")){
            return "image/jpeg";
            } 
            
            if(_hasExt(path, ".svg")){
            return "image/svg+xml";
            } 
            
            if(_hasExt(path, ".gif")){
            return "image/gif";
            } 
            
            if(_hasExt(path, ".png")){
            return "image/png";
            } 

            if(_hasExt(path, ".css")){
            return "text/css";
            } 

            if(_hasExt(path, ".html")){
            return "text/html";
            } 
            
            if(_hasExt(path, ".ico")){
            return "image/vnd.microsoft.icon";
            } 
            
            if(_hasExt(path, ".js")){
            return "text/javascript";
            } 

            return "text/plain";
        }
    };

    
#endif