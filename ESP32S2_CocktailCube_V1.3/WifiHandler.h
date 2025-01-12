/**
 * Includes all wifi functions
 *
 * @author    Florian Staeblein
 * @date      2024/01/28
 * @copyright © 2024 Florian Staeblein
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
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include <esp_log.h>
#include "Config.h"
#include "SystemHelper.h"
#include "StateMachine.h"

#if defined(WIFI_MIXER)
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

    // Updates cycle timespan in connected clients
    void UpdateCycleTimespanToClients(uint32_t clientID);

    // Updates all liquid angles in connected clients
    void UpdateLiquidAnglesToClients(uint32_t clientID);

    // Updates the web server and clients
    void Update();

    // Will be called if an web socket event occours (only internal use)
    void OnWebsocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);

  private:
    // Preferences variable
    Preferences _preferences;
    
    // Wifi settings
    bool _serverInitDone = false;
    wifi_mode_t _initWifiMode = WIFI_MODE_NULL;
    wifi_mode_t _wifiMode = WIFI_MODE_NULL;

    // SSID and Password
    const char* _ssid = MIXER_NAME;
    const char* _password = MIXER_PASSWORD;

    // Web server variables
    AsyncWebServer* _webserver;
    AsyncWebSocket* _websocket;
    AsyncEventSource* _webevents;

    // Alive counter variable
    uint32_t _lastAlive_ms = 0;

    // Starts the web server
    bool StartWebServer();

    // Updates all settings in given client
    void UpdateSettingsToClient(AsyncWebSocketClient* client);
};

//===============================================================
// Global variables
//===============================================================
extern WifiHandler Wifihandler;

#endif
#endif
