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
#include <ArduinoJson.h>
#include <Adafruit_ST77xx.h>

//===============================================================
// Defines
//===============================================================
#define APP_VERSION                       "V1.3"
#define APP_COPYRIGHT                     "2025"
#define SETTINGS_NAME                     "Settings"

// Set the value to 1 or -1 if your encoder is turning in the wrong direction
#define ENCODER_DIRECTION                 1

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
const int MixtureLiquidDashboardMax = 3;
const int MixtureLiquidCleaningMax = 4;

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
const int BarBottleMax = 5;

//===============================================================
// Structs
//===============================================================
// Config struct
struct Config
{
  bool isMixer = true;
  
  String mixerName = "CocktailCube";
  String mixerPassword = "mixer1234";

  String liquid1Name = "Liquid 1";
  String liquid2Name = "Liquid 2";
  String liquid3Name = "Liquid 3";

  int16_t liquid1AngleDegrees = 0;
  int16_t liquid2AngleDegrees = 120;
  int16_t liquid3AngleDegrees = 240;

  uint16_t tftColorStartPage = 0xFC00;
  uint16_t tftColorStartPageForeground = 0xDF9E;
  uint16_t tftColorStartPageBackground = 0xA6DC;
  uint16_t tftColorTextHeader = ST77XX_WHITE;
  uint16_t tftColorTextBody = ST77XX_WHITE;
  uint16_t tftColorInfoBoxBorder = ST77XX_BLACK;
  uint16_t tftColorInfoBoxForeground = ST77XX_BLACK;
  uint16_t tftColorInfoBoxBackground = ST77XX_WHITE;
  uint16_t tftColorMenuSelector = ST77XX_WHITE;
  uint16_t tftColorLiquid1 = ST77XX_RED;
  uint16_t tftColorLiquid2 = ST77XX_GREEN;
  uint16_t tftColorLiquid3 = ST77XX_BLUE;
  uint16_t tftColorForeground = ST77XX_WHITE;
  uint16_t tftColorBackground = ST77XX_BLACK;

  String wifiColorLiquid1 = "0xF70202";
  String wifiColorLiquid2 = "0x38F702";
  String wifiColorLiquid3 = "0x1B02F7";

  String imageLogo = "";
  String imageGlass = "";
  String imageBottle1 = "";
  String imageBottle2 = "";
  String imageBottle3 = "";
  String imageBottle4 = "";

  int16_t tftLogoPosX;
  int16_t tftLogoPosY;
  int16_t tftGlassPosX;
  int16_t tftGlassPosY;
  int16_t tftBottlePosX;
  int16_t tftBottlePosY;
};

//===============================================================
// Declarations
//===============================================================

// Loads the configuration from a JSON file
bool LoadConfig(const char* filename);

//===============================================================
// Global variables
//===============================================================
extern Config config;

#endif
