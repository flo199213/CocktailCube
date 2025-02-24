#include "http_parser.h"
/*
 * Includes all web page functions
 *
 * @author    Florian Stäblein
 * @date      2025/01/01
 * @copyright © 2025 Florian Stäblein
 */

//===============================================================
// Includes
//===============================================================
#include "WebPageHandler.h"

//===============================================================
// Constants
//===============================================================
static const char* TAG = "WebPageHandler";

//===============================================================
// Constructor
//===============================================================
WebPageHandler::WebPageHandler()
{
}

//===============================================================
// Returns true, if the handler can handle the request
//===============================================================
bool WebPageHandler::canHandle(WebServer &server, HTTPMethod method, const String &uri)
{
  // Check for URL '/control' and GET or PUT
  bool canHandle = uri.equalsIgnoreCase("/control") &&
    (method == HTTP_GET ||
    method == HTTP_PUT);

  ESP_LOGE(TAG, "Can handle request from '/control'? %s", canHandle ? "✓" : "✖");
  return canHandle;
}

//===============================================================
// Handles the request
//===============================================================
bool WebPageHandler::handle(WebServer &server, HTTPMethod method, const String &requestUri)
{
  ESP_LOGI(TAG, "Handle: request=%s", requestUri.c_str());

  if (method == HTTP_GET)
  {
    ESP_LOGI(TAG, "Handle GET");

    if (server.argName(0) == "settings")
    {
      ESP_LOGI(TAG, "Handle GET 'settings'");
      
      // Send settings
      SendSettings(server);

      ESP_LOGI(TAG, "GET: Retrieving settings successful");
      return true;
    }
    else if (server.argName(0) == "values")
    {
      ESP_LOGI(TAG, "Handle GET 'values'");

      // Send values
      SendValues(server);

      ESP_LOGI(TAG, "GET: Retrieving values successful");
      return true;
    }

    ESP_LOGE(TAG, "Unknown GET argument!");
    server.send(404, "text/plain; charset=utf-8", "Unknown GET argument!");
    return false;
  }
  else if (method == HTTP_PUT)
  {
    ESP_LOGI(TAG, "Handle PUT");

    bool result = false;
    if (server.argName(0) == LIQUID_ANGLE_1)
    {
      result = Statemachine.UpdateValuesFromWifi(eLiquid1, (int16_t)server.arg(0).toInt());
    }
    else if (server.argName(0) == LIQUID_ANGLE_2)
    {
      result = Statemachine.UpdateValuesFromWifi(eLiquid2, (int16_t)server.arg(0).toInt());
    }
    else if (server.argName(0) == LIQUID_ANGLE_3)
    {
      result = Statemachine.UpdateValuesFromWifi(eLiquid3, (int16_t)server.arg(0).toInt());
    }
    else if (server.argName(0) == CYCLE_TIMESPAN)
    {
      result = Statemachine.UpdateValuesFromWifi((uint32_t)server.arg(0).toInt());

      // Save cycle timespan value
      if (result)
      {
        Pumps.Save();
      }
    }

    if (result)
    {
      // Send HTTP OK
      server.send(200, "text/plain; charset=utf-8", "Value update success");
      return true;
    }
    
    server.send(404, "text/plain; charset=utf-8", "Value not valid");
    return false;
  }
  
  ESP_LOGE(TAG, "Unknown HTTP method '%s' or unknown argument '%s'!", method, server.argName(0));
  server.send(404, "text/plain; charset=utf-8", "Unknown HTTP method '" + String(method) + "' or unknown argument '" + server.argName(0) + "'!");
  return false;
}

//===============================================================
    // Sends all settings to the server
//===============================================================
void WebPageHandler::SendSettings(WebServer &server)
{
  int16_t liquidAngle1 = Statemachine.GetAngle(eLiquid1);
  int16_t liquidAngle2 = Statemachine.GetAngle(eLiquid2);
  int16_t liquidAngle3 = Statemachine.GetAngle(eLiquid3);
  uint32_t cycleTimepan_ms = Pumps.GetCycleTimespan();

  // Generate Json object
  String output = "[{";
  output += "\"NEED_UPDATE\":" + String(Statemachine.GetNeedUpdate()) + ","; 
  output += "\"" + String(IS_MIXER) + "\":" + String(Config.isMixer) + ",";
  output += "\"" + String(MIXER_NAME) + "\":\"" + Config.mixerName + "\",";
  output += "\"" + String(LIQUID_NAME_1) + "\":\"" + Config.liquidName1 + "\",";
  output += "\"" + String(LIQUID_NAME_2) + "\":\"" + Config.liquidName2 + "\",";
  output += "\"" + String(LIQUID_NAME_3) + "\":\"" + Config.liquidName3 + "\",";
  output += "\"" + String(LIQUID_COLOR_1) + "\":\"" + Config.liquidColor1 + "\",";
  output += "\"" + String(LIQUID_COLOR_2) + "\":\"" + Config.liquidColor2 + "\",";
  output += "\"" + String(LIQUID_COLOR_3) + "\":\"" + Config.liquidColor3 + "\",";
  output += "\"" + String(LIQUID_ANGLE_1) + "\":" + String(liquidAngle1) + ",";
  output += "\"" + String(LIQUID_ANGLE_2) + "\":" + String(liquidAngle2) + ",";
  output += "\"" + String(LIQUID_ANGLE_3) + "\":" + String(liquidAngle3) + ",";
  output += "\"" + String(CYCLE_TIMESPAN) + "\":" + String(cycleTimepan_ms);
  output += "}]";

  // Send settings
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "application/json", output);

  // Reset Json string
  output = String();
}

//===============================================================
    // Sends all values to the server
//===============================================================
void WebPageHandler::SendValues(WebServer &server)
{
  int16_t liquidAngle1 = Statemachine.GetAngle(eLiquid1);
  int16_t liquidAngle2 = Statemachine.GetAngle(eLiquid2);
  int16_t liquidAngle3 = Statemachine.GetAngle(eLiquid3);
  uint32_t cycleTimepan_ms = Pumps.GetCycleTimespan();

  // Generate Json object
  String output = "[{";
  output += "\"NEED_UPDATE\":" + String(Statemachine.GetNeedUpdate()) + ",";
  output += "\"" + String(LIQUID_ANGLE_1) + "\":" + String(liquidAngle1) + ",";
  output += "\"" + String(LIQUID_ANGLE_2) + "\":" + String(liquidAngle2) + ",";
  output += "\"" + String(LIQUID_ANGLE_3) + "\":" + String(liquidAngle3) + ",";
  output += "\"" + String(CYCLE_TIMESPAN) + "\":" + String(cycleTimepan_ms);
  output += "}]";
  
  // Send values
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "application/json", output);

  // Reset Json string
  output = String();
}
