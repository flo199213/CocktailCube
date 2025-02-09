/*
 * Includes all config defines
 *
 * @author    Florian Stäblein
 * @date      2025/01/01
 * @copyright © 2025 Florian Stäblein
 */

//===============================================================
// Includes
//===============================================================
#include "Config.h"

//===============================================================
// Converts an hex string to an uint16_t value
//===============================================================
uint16_t HexStringToUint16(const String& hexString)
{
  return (uint16_t)strtol(hexString.c_str(), nullptr, 0);
}

//===============================================================
// Loads the configuration from a JSON file
//===============================================================
bool LoadConfig(const char* filename)
{
  // Open config.txt for reading
  File configSelectFile = SPIFFS.open(filename, FILE_READ);
  if (!configSelectFile)
  {
    ESP_LOGI("config", "Failed to open config select file %s", filename);
    return false;
  }

  // Read the first line from the file
  String configFileName = configSelectFile.readStringUntil('\n');
  configSelectFile.close();

  // Remove any trailing newline or carriage return and add "/" if neccessary
  configFileName.trim();
  if (!configFileName.startsWith("/"))
  {
    configFileName = "/" + configFileName;
  }

  if (configFileName.isEmpty())
  {
    ESP_LOGI("config", "config.txt is empty");
    return false;
  }

  // Open real config file (JSON file)
  File configFile = SPIFFS.open(configFileName, FILE_READ);
  if (!configFile)
  {
    ESP_LOGI("config", "Failed to open config file %s", configFileName);
    return false;
  }

  // Deserialize JSON file
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, configFile);
  if (error)
  {
    ESP_LOGI("config", "Failed to parse JSON file: %s", error.c_str());
    configFile.close();
    return false;
  }

  // Read all config values
  config.isMixer = doc["IS_MIXER"].as<bool>();
  
  config.mixerName = doc["MIXER_NAME"].as<String>();
  config.mixerPassword = doc["MIXER_PASSWORD"].as<String>();

  config.liquid1Name = doc["LIQUID1_NAME"].as<String>();
  config.liquid2Name = doc["LIQUID2_NAME"].as<String>();
  config.liquid3Name = doc["LIQUID3_NAME"].as<String>();

  config.liquid1AngleDegrees = doc["LIQUID1ANGLE_DEGREES"];
  config.liquid2AngleDegrees = doc["LIQUID2ANGLE_DEGREES"];
  config.liquid3AngleDegrees = doc["LIQUID3ANGLE_DEGREES"];

  config.tftColorStartPage = HexStringToUint16(doc["TFT_COLOR_STARTPAGE"].as<String>());
  config.tftColorStartPageForeground = HexStringToUint16(doc["TFT_COLOR_STARTPAGE_FOREGROUND"].as<String>());
  config.tftColorStartPageBackground = HexStringToUint16(doc["TFT_COLOR_STARTPAGE_BACKGROUND"].as<String>());
  config.tftColorTextHeader = HexStringToUint16(doc["TFT_COLOR_TEXT_HEADER"].as<String>());
  config.tftColorTextBody = HexStringToUint16(doc["TFT_COLOR_TEXT_BODY"].as<String>());
  config.tftColorInfoBoxBorder = HexStringToUint16(doc["TFT_COLOR_INFOBOX_BORDER"].as<String>());
  config.tftColorInfoBoxForeground = HexStringToUint16(doc["TFT_COLOR_INFOBOX_FOREGROUND"].as<String>());
  config.tftColorInfoBoxBackground = HexStringToUint16(doc["TFT_COLOR_INFOBOX_BACKGROUND"].as<String>());
  config.tftColorMenuSelector = HexStringToUint16(doc["TFT_COLOR_MENU_SELECTOR"].as<String>());
  config.tftColorLiquid1 = HexStringToUint16(doc["TFT_COLOR_LIQUID_1"].as<String>());
  config.tftColorLiquid2 = HexStringToUint16(doc["TFT_COLOR_LIQUID_2"].as<String>());
  config.tftColorLiquid3 = HexStringToUint16(doc["TFT_COLOR_LIQUID_3"].as<String>());
  config.tftColorForeground = HexStringToUint16(doc["TFT_COLOR_FOREGROUND"].as<String>());
  config.tftColorBackground = HexStringToUint16(doc["TFT_COLOR_BACKGROUND"].as<String>());

  config.wifiColorLiquid1 = doc["WIFI_COLOR_LIQUID_1"].as<String>();
  config.wifiColorLiquid2 = doc["WIFI_COLOR_LIQUID_2"].as<String>();
  config.wifiColorLiquid3 = doc["WIFI_COLOR_LIQUID_3"].as<String>();

  config.imageLogo = doc["IMAGE_LOGO"].as<String>();
  config.imageGlass = doc["IMAGE_GLASS"].as<String>();
  config.imageBottle1 = doc["IMAGE_BOTTLE1"].as<String>();
  config.imageBottle2 = doc["IMAGE_BOTTLE2"].as<String>();
  config.imageBottle3 = doc["IMAGE_BOTTLE3"].as<String>();
  config.imageBottle4 = doc["IMAGE_BOTTLE4"].as<String>();

  config.tftLogoPosX = doc["TFT_LOGO_POS_X"];
  config.tftLogoPosY = doc["TFT_LOGO_POS_Y"];
  config.tftGlassPosX = doc["TFT_GLASS_POS_X"];
  config.tftGlassPosY = doc["TFT_GLASS_POS_Y"];
  config.tftBottlePosX = doc["TFT_BOTTLE_POS_X"];
  config.tftBottlePosY = doc["TFT_BOTTLE_POS_Y"];
  
  doc.clear();
  configFile.close();

  return true;
}
