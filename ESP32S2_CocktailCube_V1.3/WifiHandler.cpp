/*
 * Includes all wifi functions
 *
 * @author    Florian Stäblein
 * @date      2025/01/01
 * @copyright © 2025 Florian Stäblein
 */

//===============================================================
// Inlcudes
//===============================================================
#include "WifiHandler.h"

//===============================================================
// Constants
//===============================================================
static const char* TAG = "wifihandler";

//===============================================================
// Global variables
//===============================================================
WifiHandler Wifihandler;

//===============================================================
// Constructor
//===============================================================
WifiHandler::WifiHandler()
{
}

//===============================================================
// Initializes the wifi handler
//===============================================================
void WifiHandler::Begin()
{
  // Log startup info
  ESP_LOGI(TAG, "Begin initializing wifi handler");

  // Load wifi settings
  Load();

  // Set initial wifi mode
  SetWifiMode(_initWifiMode);
  
  // Log startup info
  ESP_LOGI(TAG, "Finished initializing wifi handler");
}

//===============================================================
// Load values from flash
//===============================================================
void WifiHandler::Load()
{
  if (_preferences.begin(SETTINGS_NAME, READONLY_MODE))
  {
    _initWifiMode = _preferences.getBool(KEY_WIFIMODE, true) ? WIFI_MODE_AP : WIFI_MODE_NULL;

    ESP_LOGI(TAG, "Preferences successfully loaded from '%s'", SETTINGS_NAME);
  }
  else
  {
    ESP_LOGE(TAG, "Could not open preferences '%s'", SETTINGS_NAME);
  }

  _preferences.end();
}

//===============================================================
// Save values to flash
//===============================================================
void WifiHandler::Save()
{
  if (_preferences.begin(SETTINGS_NAME, READWRITE_MODE))
  {
    _preferences.putBool(KEY_WIFIMODE, _wifiMode == WIFI_AP);

    ESP_LOGI(TAG, "Preferences successfully saved to '%s'", SETTINGS_NAME);
  }
  else
  {
    ESP_LOGE(TAG, "Could not open preferences '%s'", SETTINGS_NAME);
  }

  _preferences.end();
}

//===============================================================
// Returns the current wifi mode
//===============================================================
wifi_mode_t WifiHandler::GetWifiMode()
{
  return _wifiMode;
}

//===============================================================
// Sets the wifi mode
//===============================================================
void WifiHandler::SetWifiMode(wifi_mode_t mode)
{
  if (_wifiMode == mode)
  {
    return;
  }
  
  // Set log
  ESP_LOGI(TAG, "Set wifi mode to %s", mode == WIFI_MODE_AP ? "AP" : "OFF");

  if (mode == WIFI_MODE_AP)
  {
    IPAddress local_ip(192, 168, 1, 1);
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);

    // Set wifi TX power
    ESP_LOGI(TAG, "Set wifi TX power");
    WiFi.setTxPower(WIFI_POWER_19_5dBm);

    // Start access point
    ESP_LOGI(TAG, "Start access point");
    WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    delay(100);

    // Start web server
    _wifiMode = StartWebServer();
  }
  else
  {
    // Stop web server
    StopWebServer();

    // Deactivate Accesspoint and Wifi
    ESP_LOGI(TAG, "Deactivate Accesspoint and Wifi");
    WiFi.softAPdisconnect(true);
    
    // Set internal wifi mode
    _wifiMode = WIFI_MODE_NULL;
  }
}

//===============================================================
// Returns the amount of connected clients
//===============================================================
uint16_t WifiHandler::GetConnectedClients()
{
  return WiFi.softAPgetStationNum();
}

//===============================================================
// Updates the web server and clients
//===============================================================
void WifiHandler::Update()
{
  if (_webserver)
  {
    _webserver->handleClient();
  }
}

//===============================================================
// Starts the web server
//===============================================================
wifi_mode_t WifiHandler::StartWebServer()
{
  ESP_LOGI(TAG, "Start web server");

  // Set up mDNS responder to http://cocktailcube.local
  ESP_LOGI(TAG, "Set up mDNS responder");
  MDNS.begin(GetDNSName());
  
  // Create web server
  ESP_LOGI(TAG, "Create web server");
  _webserver = new WebServer(80);
  if (!_webserver)
  {
    return WIFI_MODE_NULL;
  }

  // Add root URL handler to web server
  ESP_LOGI(TAG, "Add root URL handler");
  _webserver->on("/", HTTP_GET, [this]()
  {
    // Stream index.html if existing
    if (SPIFFS.exists("/index.html"))
    {
      File file = SPIFFS.open("/index.html", FILE_READ);
      _webserver->streamFile(file, "text/html", 200);
      file.close();
    }

    // Send not found message
    _webserver->send(404, "text/plain; charset=utf-8", GetNotFoundMessage());
  });

  // Add system info handler to web server
  ESP_LOGI(TAG, "Add system info handler");
  _webserver->on("/systeminfo", HTTP_GET, [this]()
  {
    _webserver->sendHeader("Cache-Control", "no-cache");
    _webserver->send(200, "text/plain; charset=utf-8", Systemhelper.GetSystemInfoString());
  });

  // Add format SPIFFS handler to web server
  ESP_LOGI(TAG, "Add format SPIFFS handler");
  _webserver->on("/format", HTTP_GET, [this]()
  {
    SPIFFS.end();
    SPIFFS.format();
    _webserver->send(200, "text/plain; charset=utf-8", "FORMAT: SPIFFS sucessfully formatted. Restarting ESP...");
    delay(2000);
    ESP.restart();
  });
  
  // Add SPIFFS handler to web server (http://[WIFI_SSID].local/edit or http://192.168.1.1/edit)
  ESP_LOGI(TAG, "Add SPIFFS handler");
  _webserver->addHandler(new SPIFFSEditor());
  
  // Add web page handler to web server (http://[WIFI_SSID].local/control or http://192.168.1.1/control)
  ESP_LOGI(TAG, "Add web page handler");
  _webserver->addHandler(new WebPageHandler());
  
  // Add static files handler to web server
  ESP_LOGI(TAG, "Add static files handler");
  _webserver->serveStatic("/", SPIFFS, "/");

  // Add not found handler to web server
  ESP_LOGI(TAG, "Add not found handler");
  _webserver->onNotFound([this]()
  {
    _webserver->send(404, "text/plain; charset=utf-8", GetNotFoundMessage());
  });

  // Start web server
  ESP_LOGI(TAG, "Start web server");
  _webserver->begin();

  // Add service to MDNS
  ESP_LOGI(TAG, "Add service to MDNS");
  MDNS.addService("http", "tcp", 80);

  return WIFI_MODE_AP;
}

//===============================================================
// Stops the web server
//===============================================================
void WifiHandler::StopWebServer()
{
  if (_webserver)
  {
    // Stop web server
    ESP_LOGI(TAG, "Stop web server");
    _webserver->stop();
  }

  // Set web server to null
  ESP_LOGI(TAG, "Set web server to null");
  _webserver = NULL;
}

//===============================================================
// Returns the web domain for dns resolving
//===============================================================
String WifiHandler::GetDNSName()
{
  String mdnsName = String(WIFI_SSID);
  mdnsName.toLowerCase();
  mdnsName.trim();
  return mdnsName;
}

//===============================================================
// Returns the not found message
//===============================================================
String WifiHandler::GetNotFoundMessage()
{
  return String("Sorry, page not found! Go to 'http://") + GetDNSName() + String(".local' or 'http://192.168.1.1/'. If you want to upload files use '/edit' as sub page.");
}
