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

#define WIFI_SSID                         "CockailCube" // Should not exceed 15 characters and be a single word -> will be the dns name in lower case and without white spaces for example "http://cocktailcube.local/"
#define WIFI_PASSWORD                     "mixer1234"   // Should not exceed 15 characters

// Max config count to load (Increasing this will cost memory!)
#define MAXCONFIGS                        15

// Defines for preferences handling
#define READONLY_MODE                     true
#define READWRITE_MODE                    false
#define SETTINGS_NAME                     "Settings"
#define KEY_PREFERENCES_INITIALIZED       "NvsInit"       // Key name: Maximum string length is 15 bytes, excluding a zero terminator.

// Preferences keys for current config settings
#define KEY_CONFIGFILE                    "ConfigFile"    // Key name: Maximum string length is 15 bytes, excluding a zero terminator.
#define KEY_LEDMODE_IDLE                  "LEDIdle"       // Key name: Maximum string length is 15 bytes, excluding a zero terminator.
#define KEY_LEDMODE_DISPENSING            "LEDDispensing" // Key name: Maximum string length is 15 bytes, excluding a zero terminator.
#define KEY_ENCODER                       "Encoder"       // Key name: Maximum string length is 15 bytes, excluding a zero terminator.
#define KEY_SCREENSAVER                   "ScreenSaver"   // Key name: Maximum string length is 15 bytes, excluding a zero terminator.

// Config makro names used for loading json config files:
#define DEFAULT_CONFIGFILE                "CocktailCube.json"
#define IS_MIXER                          "IS_MIXER"
#define MIXER_NAME                        "MIXER_NAME"
#define LIQUID_NAME_1                     "LIQUID_NAME_1"
#define LIQUID_NAME_2                     "LIQUID_NAME_2"
#define LIQUID_NAME_3                     "LIQUID_NAME_3"
#define LIQUID_ANGLE_1                    "LIQUID_ANGLE_1"
#define LIQUID_ANGLE_2                    "LIQUID_ANGLE_2"
#define LIQUID_ANGLE_3                    "LIQUID_ANGLE_3"
#define LIQUID_COLOR_1                    "LIQUID_COLOR_1"
#define LIQUID_COLOR_2                    "LIQUID_COLOR_2"
#define LIQUID_COLOR_3                    "LIQUID_COLOR_3"
#define TFT_COLOR_LIQUID_1                "TFT_COLOR_LIQUID_1"
#define TFT_COLOR_LIQUID_2                "TFT_COLOR_LIQUID_2"
#define TFT_COLOR_LIQUID_3                "TFT_COLOR_LIQUID_3"
#define TFT_COLOR_STARTPAGE               "TFT_COLOR_STARTPAGE"
#define TFT_COLOR_STARTPAGE_FOREGROUND    "TFT_COLOR_STARTPAGE_FOREGROUND"
#define TFT_COLOR_STARTPAGE_BACKGROUND    "TFT_COLOR_STARTPAGE_BACKGROUND"
#define TFT_COLOR_TEXT_HEADER             "TFT_COLOR_TEXT_HEADER"
#define TFT_COLOR_TEXT_BODY               "TFT_COLOR_TEXT_BODY"
#define TFT_COLOR_INFOBOX_BORDER          "TFT_COLOR_INFOBOX_BORDER"
#define TFT_COLOR_INFOBOX_FOREGROUND      "TFT_COLOR_INFOBOX_FOREGROUND"
#define TFT_COLOR_INFOBOX_BACKGROUND      "TFT_COLOR_INFOBOX_BACKGROUND"
#define TFT_COLOR_MENU_SELECTOR           "TFT_COLOR_MENU_SELECTOR"
#define TFT_COLOR_FOREGROUND              "TFT_COLOR_FOREGROUND"
#define TFT_COLOR_BACKGROUND              "TFT_COLOR_BACKGROUND"
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
#define CYCLE_TIMESPAN                    "CYCLE_TIMESPAN"

//===============================================================
// Enums
//===============================================================
enum MixtureLiquid : int8_t
{
  eLiquid1 = 0,
  eLiquid2 = 1,
  eLiquid3 = 2,
  eLiquidAll = 3,
  eLiquidNone = 0x7F
};
const int8_t MixtureLiquidDashboardMax = 3;
const int8_t MixtureLiquidCleaningMax = 4;

enum MixerState : int8_t
{
  eMenu = 0,
  eDashboard = 1,
  eCleaning = 2,
  eReset = 3,
  eSettings = 4,
  eScreenSaver = 5,
  eBar = 6
};

enum MixerEvent : int8_t
{
  eEntry = 0,
  eMain = 1,
  eExit = 2
};

enum BarBottle : int8_t
{
  eSparklingWater = 0,
  eEmpty = 1,
  eRedWine = 2,
  eWhiteWine = 3,
  eRoseWine = 4
};
const int8_t BarBottleMax = 5;

enum MixerSetting : int8_t
{
  ePWM = 0,
  eWLAN = 1,
  eConfig = 2,
  eLEDIdle = 3,
  eLEDDispensing = 4,
  eEncoder = 5,
  eScreen = 6
};
const int8_t MixerSettingMax = 7;

enum LEDMode : int8_t
{
  eOff = 0,
  eOn = 1,
  eSlow = 2,
  eFast = 3,
  eFadingSlow = 4,
  eFadingFast = 5
};
const int8_t LEDIdleModeMax = 2;
const int8_t LEDDispensingModeMax = 6;

enum ScreensaverMode : int8_t
{
  eNone = 0,
  e2s = 1,
  e15s = 2,
  e30s = 3,
  e1min = 4,
  e5min = 5
};
const int8_t ScreensaverModeMax = 6;

//===============================================================
// Configuration class
//===============================================================
class Configuration
{
  public:
    bool isMixer;
    
    String mixerName;

    String liquidName1;
    String liquidName2;
    String liquidName3;

    int16_t liquidAngle1;
    int16_t liquidAngle2;
    int16_t liquidAngle3;

    String liquidColor1;
    String liquidColor2;
    String liquidColor3;
    
    uint16_t tftColorLiquid1;
    uint16_t tftColorLiquid2;
    uint16_t tftColorLiquid3;
    uint16_t tftColorStartPage;
    uint16_t tftColorStartPageForeground;
    uint16_t tftColorStartPageBackground;
    uint16_t tftColorTextHeader;
    uint16_t tftColorTextBody;
    uint16_t tftColorInfoBoxBorder;
    uint16_t tftColorInfoBoxForeground;
    uint16_t tftColorInfoBoxBackground;
    uint16_t tftColorMenuSelector;
    uint16_t tftColorForeground;
    uint16_t tftColorBackground;

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

    // LED settings
    LEDMode ledModeIdle = eOn;
    LEDMode ledModeDispensing = eFadingFast;

    // Encoder setting
    volatile int8_t encoderDirection = 1;

    // Screen saver setting
    int8_t screenSaverMode = e30s;

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

    // Returns the screen saver timeout in ms
    uint32_t GetScreenSaverTimeout_ms();

  private:
    // Preferences variable
    Preferences _preferences;
    
    // List of all config files
    String _files[MAXCONFIGS];
    uint8_t _fileCount = 0;
    
    // Current config index
    int16_t _currentConfigindex = -1;

    // Initializes the preferences in case of first startup ever
    void InitPreferences();

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
