/*
 * Includes all config defines
 *
 * @author    Florian Stäblein
 * @date      2025/01/01
 * @copyright © 2025 Florian Stäblein
 */

#ifndef CONFIG_H
#define CONFIG_H

//===============================================================
// Includes
//===============================================================
#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <Adafruit_ST77xx.h>

//===============================================================
// Defines
//===============================================================
#define APP_VERSION                       "V1.3"
#define APP_COPYRIGHT                     "2025"
#define SETTINGS_NAME                     "Settings"

#define WIFI_SSID                         "CockailCube" // Should not exceed 15 characters and be a single word -> will be the dns name in lower case and without white spaces for example "http://cocktailcube.local/"
#define WIFI_PASSWORD                     "mixer1234"   // Should not exceed 15 characters

// Max config count to load (Increasing this will cost memory!)
#define MAXCONFIGS                        15

// Set the value to 1 or -1 if your encoder is turning in the wrong direction
#define ENCODER_DIRECTION                 1

// Preferences key for current config file
#define KEY_CONFIGFILE                    "ConfigFile"   // Key name: Maximum string length is 15 bytes, excluding a zero terminator.

// Config makro names used for loading json config files:
#define IS_MIXER                          "IS_MIXER"
#define MIXER_NAME                        "MIXER_NAME"
#define LIQUID1_NAME                      "LIQUID1_NAME"
#define LIQUID2_NAME                      "LIQUID2_NAME"
#define LIQUID3_NAME                      "LIQUID3_NAME"
#define LIQUID1ANGLE_DEGREES              "LIQUID1ANGLE_DEGREES"
#define LIQUID2ANGLE_DEGREES              "LIQUID2ANGLE_DEGREES"
#define LIQUID3ANGLE_DEGREES              "LIQUID3ANGLE_DEGREES"
#define TFT_COLOR_STARTPAGE               "TFT_COLOR_STARTPAGE"
#define TFT_COLOR_STARTPAGE_FOREGROUND    "TFT_COLOR_STARTPAGE_FOREGROUND"
#define TFT_COLOR_STARTPAGE_BACKGROUND    "TFT_COLOR_STARTPAGE_BACKGROUND"
#define TFT_COLOR_TEXT_HEADER             "TFT_COLOR_TEXT_HEADER"
#define TFT_COLOR_TEXT_BODY               "TFT_COLOR_TEXT_BODY"
#define TFT_COLOR_INFOBOX_BORDER          "TFT_COLOR_INFOBOX_BORDER"
#define TFT_COLOR_INFOBOX_FOREGROUND      "TFT_COLOR_INFOBOX_FOREGROUND"
#define TFT_COLOR_INFOBOX_BACKGROUND      "TFT_COLOR_INFOBOX_BACKGROUND"
#define TFT_COLOR_MENU_SELECTOR           "TFT_COLOR_MENU_SELECTOR"
#define TFT_COLOR_LIQUID_1                "TFT_COLOR_LIQUID_1"
#define TFT_COLOR_LIQUID_2                "TFT_COLOR_LIQUID_2"
#define TFT_COLOR_LIQUID_3                "TFT_COLOR_LIQUID_3"
#define TFT_COLOR_FOREGROUND              "TFT_COLOR_FOREGROUND"
#define TFT_COLOR_BACKGROUND              "TFT_COLOR_BACKGROUND"
#define WIFI_COLOR_LIQUID_1               "WIFI_COLOR_LIQUID_1"
#define WIFI_COLOR_LIQUID_2               "WIFI_COLOR_LIQUID_2"
#define WIFI_COLOR_LIQUID_3               "WIFI_COLOR_LIQUID_3"
#define IMAGE_LOGO                        "IMAGE_LOGO"
#define IMAGE_GLASS                       "IMAGE_GLASS"
#define IMAGE_BOTTLE1                     "IMAGE_BOTTLE1"
#define IMAGE_BOTTLE2                     "IMAGE_BOTTLE2"
#define IMAGE_BOTTLE3                     "IMAGE_BOTTLE3"
#define IMAGE_BOTTLE4                     "IMAGE_BOTTLE4"
#define TFT_LOGO_POS_X                    "TFT_LOGO_POS_X"
#define TFT_LOGO_POS_Y                    "TFT_LOGO_POS_Y"
#define TFT_GLASS_POS_X                   "TFT_GLASS_POS_X"
#define TFT_GLASS_POS_Y                   "TFT_GLASS_POS_Y"
#define TFT_BOTTLE_POS_X                  "TFT_BOTTLE_POS_X"
#define TFT_BOTTLE_POS_Y                  "TFT_BOTTLE_POS_Y"

//===============================================================
// Enums
//===============================================================
enum MixtureLiquid : uint16_t
{
  eLiquid1 = 0,
  eLiquid2 = 1,
  eLiquid3 = 2,
  eLiquidAll = 3,
  eLiquidNone = 0xFFFF
};
const uint16_t MixtureLiquidDashboardMax = 3;
const uint16_t MixtureLiquidCleaningMax = 4;

enum MixerState : uint16_t
{
  eMenu = 0,
  eDashboard = 1,
  eCleaning = 2,
  eReset = 3,
  eSettings = 4,
  eScreenSaver = 5,
  eBar = 6,
};

enum MixerEvent : uint16_t
{
  eEntry = 0,
  eMain = 1,
  eExit = 2
};

enum BarBottle : uint16_t
{
  eSparklingWater = 0,
  eEmpty = 1,
  eRedWine = 2,
  eWhiteWine = 3,
  eRoseWine = 4,
};
const uint16_t BarBottleMax = 5;

enum MixerSetting : uint16_t
{
  ePWM = 0,
  eWLAN = 1,
  eConfig = 2,
};
const uint16_t MixerSettingMax = 3;

//===============================================================
// Configuration class
//===============================================================
class Configuration
{
  public:
    bool isMixer;
    
    String mixerName;

    String liquid1Name;
    String liquid2Name;
    String liquid3Name;

    int16_t liquid1AngleDegrees;
    int16_t liquid2AngleDegrees;
    int16_t liquid3AngleDegrees;

    uint16_t tftColorStartPage;
    uint16_t tftColorStartPageForeground;
    uint16_t tftColorStartPageBackground;
    uint16_t tftColorTextHeader;
    uint16_t tftColorTextBody;
    uint16_t tftColorInfoBoxBorder;
    uint16_t tftColorInfoBoxForeground;
    uint16_t tftColorInfoBoxBackground;
    uint16_t tftColorMenuSelector;
    uint16_t tftColorLiquid1;
    uint16_t tftColorLiquid2;
    uint16_t tftColorLiquid3;
    uint16_t tftColorForeground;
    uint16_t tftColorBackground;

    String wifiColorLiquid1;
    String wifiColorLiquid2;
    String wifiColorLiquid3;

    String imageLogo;
    String imageGlass;
    String imageBottle1;
    String imageBottle2;
    String imageBottle3;
    String imageBottle4;

    int16_t tftLogoPosX;
    int16_t tftLogoPosY;
    int16_t tftGlassPosX;
    int16_t tftGlassPosY;
    int16_t tftBottlePosX;
    int16_t tftBottlePosY;

    // Constructor
    Configuration();

    // Initializes the configuration
    bool Begin();

    // Load values from flash
    void Load();

    // Save values to flash
    void Save();

    // Returns currently loaded configuration
    String GetCurrent();

    // Increments config
    bool Increment();
    
    // Decrements config
    bool Decrement();

    // Enumerates all valid json config files
    void EnumerateConfigs();

    // Loads the configuration
    bool LoadConfig(String configFileName);
    
    // Resets the configuration to defaults
    void ResetConfig();

  private:
    // Preferences variable
    Preferences _preferences;
    
    // List of all config files
    String _files[MAXCONFIGS];
    uint8_t _fileCount = 0;
    
    // Current config index
    int16_t _currentConfigindex = -1;

    // Checks, if a file is an valid config file
    bool CheckValid(JsonDocument doc);
    
    // Loads the configuration from a JSON file
    bool LoadConfig(JsonDocument doc);

    // Converts an hex string to an uint16_t value
    bool TryHexStringToUint16(const String& hexString, uint16_t* value);
};

//===============================================================
// Global variables
//===============================================================
extern Configuration Config;

#endif
