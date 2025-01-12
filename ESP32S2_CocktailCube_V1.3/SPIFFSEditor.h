/**
 * Includes all spiffs editor functions
 *
 * @author    Florian Staeblein
 * @date      2024/01/28
 * @copyright © 2024 Florian Staeblein
 */
 
#ifndef SPIFFSEDITOR_H
#define SPIFFSEDITOR_H

//===============================================================
// Includes
//===============================================================
#include <Arduino.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include "SystemHelper.h"

//===============================================================
// SPIFFS editor class
//===============================================================
class SPIFFSEditor: public AsyncWebHandler
{
  public:
    // Constructor
    SPIFFSEditor();

    // Returns true, if the handler can handle the request
    bool canHandle(AsyncWebServerRequest* request) const override final;
    
    // Handles the request
    void handleRequest(AsyncWebServerRequest* request) override final;
    
    // Handles the upload
    void handleUpload(AsyncWebServerRequest* request, const String& filename, size_t index, uint8_t* data, size_t len, bool final) override final;
    
    // Returns false
    bool isRequestHandlerTrivial() const override final
    {
      return false;
    }
    
  private:
    uint32_t _startTime;
};

#endif
