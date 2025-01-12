/**
 * Includes all wifi functions
 *
 * @author    Florian Staeblein
 * @date      2024/01/28
 * @copyright © 2024 Florian Staeblein
 */

//===============================================================
// Inlcudes
//===============================================================
#include "WifiHandler.h"

//===============================================================
// Constants
//===============================================================
static const char* TAG = "wifihandler";

#if defined(WIFI_MIXER)

//===============================================================
// Global variables
//===============================================================
WifiHandler Wifihandler;

//===============================================================
// Will be called if an web socket event occours
//===============================================================
void onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len)
{
  Wifihandler.OnWebsocketEvent(server, client, type, arg, data, len);
}

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
  if (_preferences.begin(SETTINGS_NAME, true))
  {
    _initWifiMode = _preferences.getBool(KEY_WIFIMODE, false) ? WIFI_MODE_AP : WIFI_MODE_NULL;
    _preferences.end();

    ESP_LOGI(TAG, "Preferences successfully loaded from '%s'", SETTINGS_NAME);
  }
  else
  {
    ESP_LOGE(TAG, "Could not open preferences '%s'", SETTINGS_NAME);
  }
}

//===============================================================
// Save values to flash
//===============================================================
void WifiHandler::Save()
{
  if (_preferences.begin(SETTINGS_NAME, false))
  {
    _preferences.putBool(KEY_WIFIMODE, _wifiMode == WIFI_AP);
    _preferences.end();

    ESP_LOGI(TAG, "Preferences successfully saved to '%s'", SETTINGS_NAME);
  }
  else
  {
    ESP_LOGE(TAG, "Could not open preferences '%s'", SETTINGS_NAME);
  }
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
    WiFi.softAP(_ssid, _password);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    delay(100);

    // Check for first call
    if (!_serverInitDone)
    {
      // Start web server
      _serverInitDone = StartWebServer();

      // Set internal wifi mode if success
      _wifiMode = _serverInitDone ? WIFI_MODE_AP : WIFI_MODE_NULL;
    }
    else
    {
      _wifiMode = WIFI_MODE_AP;
    }
  }
  else
  {
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
// Updates cycle timespan in connected clients
//===============================================================
void WifiHandler::UpdateCycleTimespanToClients(uint32_t clientID)
{
  if (_webevents == NULL)
  {
    return;
  }

  uint32_t cycleTimepan_ms = Pumps.GetCycleTimespan();
  _webevents->send((String(clientID) + ":" + String(cycleTimepan_ms)).c_str(), "CYCLE_TIMESPAN");
}

//===============================================================
// Updates all liquid angles in connected clients
//===============================================================
void WifiHandler::UpdateLiquidAnglesToClients(uint32_t clientID)
{
  if (!_webevents)
  {
    return;
  }

  int16_t angle1 = Statemachine.GetAngle(eLiquid1);
  int16_t angle2 = Statemachine.GetAngle(eLiquid2);
  int16_t angle3 = Statemachine.GetAngle(eLiquid3);

  // Send events from variables to all connected websockets
  _webevents->send((String(clientID) + ":" + String(angle1) + "," + String(angle2)  + "," + String(angle3)).c_str(), "LIQUID_ANGLES");
}

//===============================================================
// Updates the web server and clients
//===============================================================
void WifiHandler::Update()
{
  if (_websocket)
  {
    // Clean websocket clients
    _websocket->cleanupClients();
  }

  if (_webevents)
  {
    // Send alive signal every second
    if (millis() - _lastAlive_ms > 1000)
    {
      //_webevents->send("", "ALIVE");
      UpdateLiquidAnglesToClients(0); // Use liquid angles instead (long time sync and alive in once)
      _lastAlive_ms = millis();
    }
  }
}

//===============================================================
// Will be called if an web socket event occours
//===============================================================
void WifiHandler::OnWebsocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len)
{
  // Client connected
  if (type == WS_EVT_CONNECT)
  {
    // Send all static settings to mixer on websocket connect
    client->printf("Connected Client Number %u", (uint16_t)client->id());
    UpdateSettingsToClient(client);
    client->ping();
    ESP_LOGI(TAG, "Client connected");
  }
  //else if (type == WS_EVT_DISCONNECT)
  //else if (type == WS_EVT_ERROR)
  //else if (type == WS_EVT_PONG)
  else if (type == WS_EVT_DATA)
  {
    AwsFrameInfo* info = (AwsFrameInfo*)arg;
    String msg = "";
    if (info->final &&
      info->index == 0 &&
      info->len == len)
    {
      // The whole message is in a single frame and we got all of it's data
      if (info->opcode == WS_TEXT)
      {
        for (size_t index = 0; index < info->len; index++)
        {
          msg += (char)data[index];
        }

        // Update last user interaction
        Systemhelper.SetLastUserAction();

        if (msg.startsWith("FULLUPDATE"))
        {
          // Send all static settings to mixer on websocket connect
          client->text("Valid Fullupdate received!");
          UpdateSettingsToClient(client);
          client->ping();
        }
        else if (msg.startsWith("LIQUID_INCREMENT:"))
        {
          // Split the message by a pre-defined delimiter
          int delimiter = msg.indexOf(":");
          int delimiter_1 = msg.indexOf(",", delimiter + 1);

          String liquid_String = msg.substring(delimiter + 1, delimiter_1);
          String liquidAngle_String = msg.substring(delimiter_1 + 1);

          MixtureLiquid liquid = (MixtureLiquid)liquid_String.toInt();
          int16_t liquidIncrements_Degrees = (int16_t)liquidAngle_String.toInt();

          if (Statemachine.UpdateValuesFromWifi((uint32_t)client->id(), liquid, liquidIncrements_Degrees))
          {
            client->text("Valid liquid increments received!");
          }
          else
          {
            client->text("Invalid liquid increments received!");
          }
        }
        else if (msg.startsWith("CYCLE_TIMESPAN:"))
        {
          String cycleTimespan_String = msg.substring(msg.indexOf(":") + 1);
          uint32_t cycleTimespan_ms = cycleTimespan_String.toInt();

          if (Statemachine.UpdateValuesFromWifi((uint32_t)client->id(), cycleTimespan_ms))
          {
            client->text("Valid cycle timespan received!");
          }
          else
          {
            client->text("Invalid cycle timespan received!");
          }
        }
        else if (msg.startsWith("SAVE"))
        {
          if (Statemachine.UpdateValuesFromWifi((uint32_t)client->id(), true))
          {
            client->text("Valid save received!");
          }
          else
          {
            client->text("Invalid save received!");
          }
        }
      }
    }
  }
}

//===============================================================
// Starts the web server
//===============================================================
bool WifiHandler::StartWebServer()
{
  ESP_LOGI(TAG, "Start web server");

  // Set up mDNS responder to mixer name, e.g. http://mixer.local
  ESP_LOGI(TAG, "Set up mDNS responde");
  String mdnsName = MIXER_NAME;
  mdnsName.toLowerCase();
  mdnsName.trim();
  MDNS.begin(mdnsName);

  // Create web server
  ESP_LOGI(TAG, "Create web server");
  _webserver = new AsyncWebServer(80);
  if (!_webserver)
  {
    return false;
  }

  // Create web socket
  ESP_LOGI(TAG, "Create web socket");
  _websocket = new AsyncWebSocket("/websocket");
  if (!_websocket)
  {
    return false;
  }

  // Create web events
  ESP_LOGI(TAG, "Create web events");
  _webevents = new AsyncEventSource("/events");
  if (!_webevents)
  {
    return false;
  }
  
  // Add root URL handler to web server
  ESP_LOGI(TAG, "Add root URL handler");
  _webserver->on("/", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  // Add system info URL handler to web server
  ESP_LOGI(TAG, "Add system info URL handler");
  _webserver->on("/systeminfo", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(200, "text/plain", Systemhelper.GetSystemInfoString());
  });

  // Add SPIFFS handler to web server
  ESP_LOGI(TAG, "Add SPIFFS handler");
  _webserver->addHandler(new SPIFFSEditor());

  // Add not found handler to web server
  ESP_LOGI(TAG, "Add not found handler");
  _webserver->onNotFound([](AsyncWebServerRequest *request)
  {
    String mixerName = MIXER_NAME;
    mixerName.toLowerCase();
    mixerName.trim();
    request->send(404, "text/plain", "Sorry, page not found! Go to 'http://" + mixerName + ".local' or 'http://192.168.1.1/'");
  });

  // Add websocket handler to web server
  ESP_LOGI(TAG, "Add websocket handler");
  _websocket->onEvent(onWsEvent);
  _webserver->addHandler(_websocket);

  // Add event handler to web server
  ESP_LOGI(TAG, "Add event handler");
  _webserver->addHandler(_webevents);

  // Add static files handler to web server
  ESP_LOGI(TAG, "Add static files handler");
  _webserver->serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  
  // Start web server
  ESP_LOGI(TAG, "Start web server");
  _webserver->begin();

  // Add service to MDNS
  ESP_LOGI(TAG, "Add service to MDNS");
  MDNS.addService("http", "tcp", 80);

  return true;
}

//===============================================================
// Updates all settings in given client
//===============================================================
void WifiHandler::UpdateSettingsToClient(AsyncWebSocketClient* client)
{
  if (!client)
  {
    return;
  }

  int16_t angle1 = Statemachine.GetAngle(eLiquid1);
  int16_t angle2 = Statemachine.GetAngle(eLiquid2);
  int16_t angle3 = Statemachine.GetAngle(eLiquid3);
  uint32_t cycleTimepan_ms = Pumps.GetCycleTimespan();

  client->printf("CLIENT_ID:%s", String(client->id()).c_str());
  client->printf("MIXER_NAME:%s", MIXER_NAME);
  client->printf("LIQUID_NAMES:%s,%s,%s", LIQUID1_NAME, LIQUID2_NAME, LIQUID3_NAME);
  client->printf("LIQUID_COLORS:%s,%s,%s", String(WIFI_COLOR_LIQUID_1).c_str(), String(WIFI_COLOR_LIQUID_2).c_str(), String(WIFI_COLOR_LIQUID_3).c_str());
  client->printf("LIQUID_ANGLES:%s,%s,%s", String(angle1).c_str(), String(angle2).c_str(), String(angle3).c_str());
  client->printf("CYCLE_TIMESPAN:%s", String(cycleTimepan_ms).c_str());
}

#endif
