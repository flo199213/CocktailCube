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
// Constants
//===============================================================
static const char* TAG = "config";

//===============================================================
// Global variables
//===============================================================
Configuration Config;

//===============================================================
// Constructor
//===============================================================
Configuration::Configuration()
{
}

//===============================================================
// Initializes the configuration
//===============================================================
bool Configuration::Begin()
{
  // Log startup info
  ESP_LOGI(TAG, "Begin initializing configuration");

  // Reset configuration
  ResetConfig();

  // Enumerate all config files from file system
  EnumerateConfigs();

  // Load configuration settings
  Load();
  
  // Load initial configuration
  bool result = LoadConfig(GetCurrent());
  
  // Log startup info
  ESP_LOGI(TAG, "Finished initializing configuration");

  return result;
}

//===============================================================
// Load values from flash
//===============================================================
void Configuration::Load()
{
  if (_preferences.begin(SETTINGS_NAME, true))
  {
    String currentConfigFileName = _preferences.getString(KEY_CONFIGFILE, String(DEFAULT_CONFIGFILE));
    ledModeIdle = (LEDMode)_preferences.getUShort(KEY_LEDMODE_IDLE, eOn);
    ledModeDispensing = (LEDMode)_preferences.getUShort(KEY_LEDMODE_DISPENSING, eFadingFast);
    _preferences.end();

    _currentConfigindex = -1;
    for (uint8_t index = 0; index < _fileCount; index++)
    {
      if (currentConfigFileName == _files[index])
      {
        _currentConfigindex = index;
        break;
      }
    }

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
void Configuration::Save()
{
  if (_preferences.begin(SETTINGS_NAME, false))
  {
    _preferences.putString(KEY_CONFIGFILE, GetCurrent());
    _preferences.putUShort(KEY_LEDMODE_IDLE, ledModeIdle);
    _preferences.putUShort(KEY_LEDMODE_DISPENSING, ledModeDispensing);
    _preferences.end();

    ESP_LOGI(TAG, "Preferences successfully saved to '%s'", SETTINGS_NAME);
  }
  else
  {
    ESP_LOGE(TAG, "Could not open preferences '%s'", SETTINGS_NAME);
  }
}

//===============================================================
// Returns currently loaded configuration
//===============================================================
String Configuration::GetCurrent()
{
  if (_currentConfigindex >= 0 && 
    _currentConfigindex < _fileCount)
  {
    return _files[_currentConfigindex];
  }

  return "default";
}

//===============================================================
// Increments config
//===============================================================
bool Configuration::Increment()
{
  if (_fileCount == 0)
  {
    return false;
  }

  _currentConfigindex++;
  
  if (_currentConfigindex >= _fileCount)
  {
    _currentConfigindex = 0;
  }
  
  return true;
};

//===============================================================
// Decrements config
//===============================================================
bool Configuration::Decrement()
{
  if (_fileCount == 0)
  {
    return false;
  }

  _currentConfigindex--;
  
  if (_currentConfigindex < 0)
  {
    _currentConfigindex = _fileCount - 1;
  }

  return true;
}

//===============================================================
// Enumerates all valid json config files
//===============================================================
void Configuration::EnumerateConfigs()
{
  ESP_LOGI(TAG, "Started enumerating config files");

  // Reset all config file names
  _fileCount = 0;
  for (uint8_t index = 0; index < MAXCONFIGS; index++)
  {
    _files[index] = "";
  }

  // Open root directory
  File rootDirectory = SPIFFS.open("/", FILE_READ);

  // Check if existing
  if (!rootDirectory)
  {
    ESP_LOGI(TAG, "No root directory found");
    return;
  }

  // Open first file
  File entry = rootDirectory.openNextFile();;

  // Iterate through all files and save valid config file names
  String currentName = "";
  while (entry)
  {
    if (_fileCount >= MAXCONFIGS)
    {
      break;
    }

    // Check for json file and valid config
    JsonDocument doc;
    if (!entry.isDirectory() &&
      String(entry.name()).endsWith(".json") &&
      deserializeJson(doc, entry).code() == DeserializationError::Ok &&
      CheckValid(doc))
    {
      ESP_LOGI(TAG, "Found: %s", entry.name());
      _files[_fileCount++] = String(entry.name());
    }
    
    // Clear json document
    doc.clear();

    // Get next file
    entry = rootDirectory.openNextFile();
  }

  // Close root directory
  rootDirectory.close();

  ESP_LOGI(TAG, "Finished enumerating config files. Found %d files.", _fileCount);
}

//===============================================================
// Loads the configuration
//===============================================================
bool Configuration::LoadConfig(String configFileName)
{
  ESP_LOGI(TAG, "Starting load config file '%s'", configFileName.c_str());

  String configFilePath = configFileName;
  if (!configFileName.startsWith("/"))
  {
    configFilePath = "/" + configFileName;
  }

  // Open config file (JSON file)
  File configFile = SPIFFS.open(configFilePath, FILE_READ);
  if (!configFile)
  {
    ESP_LOGE(TAG, "Failed to open config file '%s'", configFilePath.c_str());
    return false;
  }

  // Deserialize JSON file
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, configFile);
  if (error)
  {
    ESP_LOGE(TAG, "Failed to parse JSON file: '%s'", error.c_str());
    configFile.close();
    return false;
  }

  // Load configfrom json document
  bool result = LoadConfig(doc);

  // Clear json document and close file
  doc.clear();
  configFile.close();
  
  ESP_LOGI(TAG, "Finished load config file '%s'", configFileName.c_str());
  return result;
}

//===============================================================
// Resets the configuration to defaults
//===============================================================
void Configuration::ResetConfig()
{
  Config.isMixer = true;
  Config.mixerName = "CocktailCube";
  Config.liquidName1 = "Liquid 1";
  Config.liquidName2 = "Liquid 2";
  Config.liquidName3 = "Liquid 3";
  Config.liquidAngle1 = 0;
  Config.liquidAngle2 = 120;
  Config.liquidAngle3 = 240;
  Config.liquidColor1 = "#FE5000";
  Config.liquidColor2 = "#01FFFF";
  Config.liquidColor3 = "#00E784";
  Config.tftColorLiquid1 = 0xFC00;
  Config.tftColorLiquid2 = 0x0F1F;
  Config.tftColorLiquid3 = 0x0390;
  Config.tftColorStartPage = 0xFC00;
  Config.tftColorStartPageForeground = 0xDF9E;
  Config.tftColorStartPageBackground = 0xA6DC;
  Config.tftColorTextHeader = 0xFC00;
  Config.tftColorTextBody = 0xFFFF;
  Config.tftColorInfoBoxBorder = 0xFC00;
  Config.tftColorInfoBoxForeground = 0xFC00;
  Config.tftColorInfoBoxBackground = 0xFFFF;
  Config.tftColorMenuSelector = 0xFC00;
  Config.tftColorForeground = 0xFFFF;
  Config.tftColorBackground = 0x0000;
  Config.imageLogo = "";
  Config.imageGlass = "";
  Config.imageBottle1 = "";
  Config.imageBottle2 = "";
  Config.imageBottle3 = "";
  Config.imageBottle4 = "";
  Config.tftLogoPosX = 0;
  Config.tftLogoPosY = 0;
  Config.tftGlassPosX = 0;
  Config.tftGlassPosY = 0;
  Config.tftBottlePosX = 0;
  Config.tftBottlePosY = 0;
}

//===============================================================
// Checks, if a file is an valid config file
//===============================================================
bool Configuration::CheckValid(JsonDocument doc)
{ 
  // Check all config values
  bool valid = true;
  
  // Check mixer config file state
  valid |= doc[IS_MIXER].is<bool>();

  // Check mixer name
  valid |= doc[MIXER_NAME].is<String>() && doc[MIXER_NAME].as<String>().length() <= 15;

  // Check liquid names
  valid |= doc[LIQUID_NAME_1].is<String>() && doc[LIQUID_NAME_1].as<String>().length() <= 10;
  valid |= doc[LIQUID_NAME_2].is<String>() && doc[LIQUID_NAME_2].as<String>().length() <= 10;
  valid |= doc[LIQUID_NAME_3].is<String>() && doc[LIQUID_NAME_3].as<String>().length() <= 10;
  
  // Check default liquid angles
  valid |= doc[LIQUID_ANGLE_1].is<int16_t>();
  valid |= doc[LIQUID_ANGLE_2].is<int16_t>();
  valid |= doc[LIQUID_ANGLE_3].is<int16_t>();

  // Check liquid wifi colors
  valid |= doc[LIQUID_COLOR_1].is<String>();
  valid |= doc[LIQUID_COLOR_2].is<String>();
  valid |= doc[LIQUID_COLOR_3].is<String>();

  // Check and convert TFT colors
  uint16_t dummyValue;
  valid |= doc[TFT_COLOR_LIQUID_1].is<String>() && TryHexStringToUint16(doc[TFT_COLOR_LIQUID_1].as<String>(), &dummyValue);
  valid |= doc[TFT_COLOR_LIQUID_2].is<String>() && TryHexStringToUint16(doc[TFT_COLOR_LIQUID_2].as<String>(), &dummyValue);
  valid |= doc[TFT_COLOR_LIQUID_3].is<String>() && TryHexStringToUint16(doc[TFT_COLOR_LIQUID_3].as<String>(), &dummyValue);
  valid |= doc[TFT_COLOR_STARTPAGE].is<String>() && TryHexStringToUint16(doc[TFT_COLOR_STARTPAGE].as<String>(), &dummyValue);
  valid |= doc[TFT_COLOR_STARTPAGE_FOREGROUND].is<String>() && TryHexStringToUint16(doc[TFT_COLOR_STARTPAGE_FOREGROUND].as<String>(), &dummyValue);
  valid |= doc[TFT_COLOR_STARTPAGE_BACKGROUND].is<String>() && TryHexStringToUint16(doc[TFT_COLOR_STARTPAGE_BACKGROUND].as<String>(), &dummyValue);
  valid |= doc[TFT_COLOR_TEXT_HEADER].is<String>() && TryHexStringToUint16(doc[TFT_COLOR_TEXT_HEADER].as<String>(), &dummyValue);
  valid |= doc[TFT_COLOR_TEXT_BODY].is<String>() && TryHexStringToUint16(doc[TFT_COLOR_TEXT_BODY].as<String>(), &dummyValue);
  valid |= doc[TFT_COLOR_INFOBOX_BORDER].is<String>() && TryHexStringToUint16(doc[TFT_COLOR_INFOBOX_BORDER].as<String>(), &dummyValue);
  valid |= doc[TFT_COLOR_INFOBOX_FOREGROUND].is<String>() && TryHexStringToUint16(doc[TFT_COLOR_INFOBOX_FOREGROUND].as<String>(), &dummyValue);
  valid |= doc[TFT_COLOR_INFOBOX_BACKGROUND].is<String>() && TryHexStringToUint16(doc[TFT_COLOR_INFOBOX_BACKGROUND].as<String>(), &dummyValue);
  valid |= doc[TFT_COLOR_MENU_SELECTOR].is<String>() && TryHexStringToUint16(doc[TFT_COLOR_MENU_SELECTOR].as<String>(), &dummyValue);
  valid |= doc[TFT_COLOR_FOREGROUND].is<String>() && TryHexStringToUint16(doc[TFT_COLOR_FOREGROUND].as<String>(), &dummyValue);
  valid |= doc[TFT_COLOR_BACKGROUND].is<String>() && TryHexStringToUint16(doc[TFT_COLOR_BACKGROUND].as<String>(), &dummyValue);

  // Check image file names
  valid |= doc[IMAGE_LOGO].is<String>();
  valid |= doc[IMAGE_GLASS].is<String>();
  valid |= doc[IMAGE_BOTTLE1].is<String>();
  valid |= doc[IMAGE_BOTTLE2].is<String>();
  valid |= doc[IMAGE_BOTTLE3].is<String>();
  valid |= doc[IMAGE_BOTTLE4].is<String>();

  // Check image positions
  valid |= doc[TFT_LOGO_POS_X].is<int16_t>();
  valid |= doc[TFT_LOGO_POS_Y].is<int16_t>();
  valid |= doc[TFT_GLASS_POS_X].is<int16_t>();
  valid |= doc[TFT_GLASS_POS_Y].is<int16_t>();
  valid |= doc[TFT_BOTTLE_POS_X].is<int16_t>();
  valid |= doc[TFT_BOTTLE_POS_Y].is<int16_t>();
  
  return valid;
}

//===============================================================
// Loads the configuration from a JSON file
//===============================================================
bool Configuration::LoadConfig(JsonDocument doc)
{
  // Check if document is valid
  if (!CheckValid(doc))
  {
    ESP_LOGE(TAG, "Config file not valid");
    return false;
  }
  
  // Read mixer config file state
  Config.isMixer = doc[IS_MIXER].as<bool>();
  
  // Read mixer name and password
  Config.mixerName = doc[MIXER_NAME].as<String>();

  // Read liquid names
  Config.liquidName1 = doc[LIQUID_NAME_1].as<String>();
  Config.liquidName2 = doc[LIQUID_NAME_2].as<String>();
  Config.liquidName3 = doc[LIQUID_NAME_3].as<String>();

  // Read default liquid angles
  Config.liquidAngle1 = doc[LIQUID_ANGLE_1].as<int16_t>();
  Config.liquidAngle2 = doc[LIQUID_ANGLE_2].as<int16_t>();
  Config.liquidAngle3 = doc[LIQUID_ANGLE_3].as<int16_t>();

  // Read wifi colors
  Config.liquidColor1 = doc[LIQUID_COLOR_1].as<String>();
  Config.liquidColor2 = doc[LIQUID_COLOR_2].as<String>();
  Config.liquidColor3 = doc[LIQUID_COLOR_3].as<String>();

  // Read and convert TFT colors
  TryHexStringToUint16(doc[TFT_COLOR_LIQUID_1].as<String>(), &Config.tftColorLiquid1);
  TryHexStringToUint16(doc[TFT_COLOR_LIQUID_2].as<String>(), &Config.tftColorLiquid2);
  TryHexStringToUint16(doc[TFT_COLOR_LIQUID_3].as<String>(), &Config.tftColorLiquid3);
  TryHexStringToUint16(doc[TFT_COLOR_STARTPAGE].as<String>(), &Config.tftColorStartPage);
  TryHexStringToUint16(doc[TFT_COLOR_STARTPAGE_FOREGROUND].as<String>(), &Config.tftColorStartPageForeground);
  TryHexStringToUint16(doc[TFT_COLOR_STARTPAGE_BACKGROUND].as<String>(), &Config.tftColorStartPageBackground);
  TryHexStringToUint16(doc[TFT_COLOR_TEXT_HEADER].as<String>(), &Config.tftColorTextHeader);
  TryHexStringToUint16(doc[TFT_COLOR_TEXT_BODY].as<String>(), &Config.tftColorTextBody);
  TryHexStringToUint16(doc[TFT_COLOR_INFOBOX_BORDER].as<String>(), &Config.tftColorInfoBoxBorder);
  TryHexStringToUint16(doc[TFT_COLOR_INFOBOX_FOREGROUND].as<String>(), &Config.tftColorInfoBoxForeground);
  TryHexStringToUint16(doc[TFT_COLOR_INFOBOX_BACKGROUND].as<String>(), &Config.tftColorInfoBoxBackground);
  TryHexStringToUint16(doc[TFT_COLOR_MENU_SELECTOR].as<String>(), &Config.tftColorMenuSelector);
  TryHexStringToUint16(doc[TFT_COLOR_FOREGROUND].as<String>(), &Config.tftColorForeground);
  TryHexStringToUint16(doc[TFT_COLOR_BACKGROUND].as<String>(), &Config.tftColorBackground);

  // Read image file names
  Config.imageLogo = doc[IMAGE_LOGO].as<String>();
  Config.imageGlass = doc[IMAGE_GLASS].as<String>();
  Config.imageBottle1 = doc[IMAGE_BOTTLE1].as<String>();
  Config.imageBottle2 = Config.isMixer ? "" : doc[IMAGE_BOTTLE2].as<String>();
  Config.imageBottle3 = Config.isMixer ? "" :  doc[IMAGE_BOTTLE3].as<String>();
  Config.imageBottle4 = Config.isMixer ? "" :  doc[IMAGE_BOTTLE4].as<String>();
  
  // Read image positions
  Config.tftLogoPosX = doc[TFT_LOGO_POS_X].as<int16_t>();
  Config.tftLogoPosY = doc[TFT_LOGO_POS_Y].as<int16_t>();
  Config.tftGlassPosX = doc[TFT_GLASS_POS_X].as<int16_t>();
  Config.tftGlassPosY = doc[TFT_GLASS_POS_Y].as<int16_t>();
  Config.tftBottlePosX = doc[TFT_BOTTLE_POS_X].as<int16_t>();
  Config.tftBottlePosY = doc[TFT_BOTTLE_POS_Y].as<int16_t>();
  
  return true;
}

//===============================================================
// Converts an hex string to an uint16_t value
//===============================================================
bool Configuration::TryHexStringToUint16(const String& hexString, uint16_t* value)
{
  *value = 0;

  char* endptr;
  const char* str = hexString.c_str();
  int32_t result = strtol(str, &endptr, 0);

  // Check for converting error
  if (endptr == str)
  {
    return false;
  }
  
  // Check for correct uint16_t range
  if (result < 0 || result > 65535)
  {
    return false;
  }

  *value = (uint16_t)result;
  return true;
}
