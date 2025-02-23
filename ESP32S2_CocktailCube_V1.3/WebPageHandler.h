/*
 * Includes all web page functions
 *
 * @author    Florian Stäblein
 * @date      2025/01/01
 * @copyright © 2025 Florian Stäblein
 */

#ifndef WEBPAGEHANDLER_H
#define WEBPAGEHANDLER_H

//===============================================================
// Includes
//===============================================================
#include <Arduino.h>
#include <WebServer.h>
#include "StateMachine.h"
#include "PumpDriver.h"

//===============================================================
// SPIFFS editor class
//===============================================================
class WebPageHandler: public RequestHandler
{
  public:
    // Constructor
    WebPageHandler();

    // Returns true, if the handler can handle the request
    bool canHandle(WebServer &server, HTTPMethod method, const String &uri) override final;

    // Handles the request
    bool handle(WebServer &server, HTTPMethod method, const String &requestUri) override final;

  private:
    // Sends all settings to the server
    void SendSettings(WebServer &server);

    // Sends all values to the server
    void SendValues(WebServer &server);
};

#endif
