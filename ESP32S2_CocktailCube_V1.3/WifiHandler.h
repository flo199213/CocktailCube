/*
 * Includes all wifi functions
 *
 * @author    Florian Stäblein
 * @date      2025/01/01
 * @copyright © 2025 Florian Stäblein
 */
 
#ifndef WIFIHANDLER_H
#define WIFIHANDLER_H

//===============================================================
// Includes
//===============================================================
#include <Arduino.h>
#include <Preferences.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <esp_log.h>
#include "Config.h"
#include "SystemHelper.h"
#include "StateMachine.h"
#include "SPIFFSEditor.h"

//===============================================================
// Defines
//===============================================================
#define KEY_WIFIMODE      "WifiMode"   // Key name: Maximum string length is 15 bytes, excluding a zero terminator.

//===============================================================
// Class for wifi handling
//===============================================================
class WifiHandler
{
  public:
    // Constructor
    WifiHandler();

    // Initializes the wifi handler
    void Begin();

    // Load values from flash
    void Load();

    // Save values to flash
    void Save();

    // Returns the current wifi mode
    wifi_mode_t GetWifiMode();

    // Sets the wifi mode
    void SetWifiMode(wifi_mode_t mode);

    // Returns the amount of connected clients
    uint16_t GetConnectedClients();

    // Updates the web server and clients
    void Update();

    // Returns the internal websever (only internal use)
    WebServer* GetWebServer() { return _webserver; }

  private:
    // Preferences variable
    Preferences _preferences;
    
    // Wifi settings
    wifi_mode_t _initWifiMode = WIFI_MODE_NULL;
    wifi_mode_t _wifiMode = WIFI_MODE_NULL;

    // Web server variables
    WebServer* _webserver;

    // Alive counter variable
    uint32_t _lastAlive_ms = 0;

    // Starts the web server
    wifi_mode_t StartWebServer();

    // Stops the web server
    void StopWebServer();
};

//===============================================================
// Global variables
//===============================================================
extern WifiHandler Wifihandler;

#endif
