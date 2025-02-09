/*
 * Includes all spiffs editor functions
 *
 * @author    Florian Stäblein
 * @date      2025/01/01
 * @copyright © 2025 Florian Stäblein
 */

#ifndef SPIFFSEDITOR_H
#define SPIFFSEDITOR_H

//===============================================================
// Includes
//===============================================================
#include <Arduino.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include "SystemHelper.h"

//===============================================================
// SPIFFS editor class
//===============================================================
class SPIFFSEditor: public RequestHandler
{
  public:
    // Constructor
    SPIFFSEditor();

    // Returns true, if the handler can handle the request
    bool canHandle(WebServer &server, HTTPMethod method, const String &uri) override final;

    // Returns true if the handler can upload the file
    bool canUpload(WebServer &server, const String &uri) override final;

    // Handles the request
    bool handle(WebServer &server, HTTPMethod method, const String &requestUri) override final;

    // Handles the upload
    void upload(WebServer &server, const String &requestUri, HTTPUpload &upload) override final;
  
  private:
    File _fsUploadFile;
    String _fileName = String();
    bool _uploadSuccess = false;
};

#endif
