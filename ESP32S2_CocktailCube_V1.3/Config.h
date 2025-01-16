/**
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
#include <Adafruit_ST77xx.h>

//===============================================================
// Defines
//===============================================================
#define APP_VERSION                       "V1.3"
#define SETTINGS_NAME                     "Settings"

// Define one of these or none for generic colors
//#define APEROLIKER
//#define APEROLIC
//#define HUGOLIKER
#define WILDBERRY

// This means that the ESP will wait 2 seconds each time it is started
// because the start of the serial debug output on ESP32-S2 takes this time
#define DEBUG_MIXER     // Uncomment for debug build

// Using the mixer without wifi makes the firmware more stable
#define WIFI_MIXER      // Uncomment for wifi usage

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
};

enum MixerEvent : uint16_t
{
  eEntry = 0,
  eMain = 1,
  eExit = 2
};

//===============================================================
// Variant specific settings:
//===============================================================
#if defined(APEROLIKER)

// Wifi Settings
#define MIXER_NAME                        "APEROLiker"    // Should not exceed 15 characters and be a single word -> will be the dns name in lower case and without white spaces for example "http://aperoliker.local/"
#define MIXER_PASSWORD                    "mixer1234"     // Should not exceed 15 characters

// Draw setting
#define LIQUID1_NAME                      "Aperol"        // Should not exceed 8 characters
#define LIQUID2_NAME                      "Soda"          // Should not exceed 8 characters
#define LIQUID3_NAME                      "Prosecco"      // Should not exceed 8 characters

// Mixture default setting
// Aperol: 34%, Soda: 16%, Prosecco: 50% (Official Aperol recipe)
#define LIQUID1ANGLE_DEGREES              0     //   0° -> 120° ( / 360°) = 33,33%
#define LIQUID2ANGLE_DEGREES              120   // 120° -> 177° ( / 360°) = 15,83%
#define LIQUID3ANGLE_DEGREES              177   // 177° -> 360° ( / 360°) = 50,84%

// Color defines
#define TFT_COLOR_STARTPAGE               ST77XX_ORANGE
#define TFT_COLOR_STARTPAGE_FOREGROUND    0xDF9E
#define TFT_COLOR_STARTPAGE_BACKGROUND    0xA6DC
#define TFT_COLOR_TEXT_HEADER             ST77XX_ORANGE
#define TFT_COLOR_TEXT_BODY               ST77XX_WHITE
#define TFT_COLOR_INFOBOX_BORDER          ST77XX_ORANGE
#define TFT_COLOR_INFOBOX_FOREGROUND      ST77XX_ORANGE
#define TFT_COLOR_INFOBOX_BACKGROUND      ST77XX_WHITE
#define TFT_COLOR_MENU_SELECTOR           ST77XX_ORANGE
#define TFT_COLOR_LIQUID_1                ST77XX_ORANGE
#define TFT_COLOR_LIQUID_2                0x0F1F
#define TFT_COLOR_LIQUID_3                0x0390
#define TFT_COLOR_FOREGROUND              ST77XX_WHITE
#define TFT_COLOR_BACKGROUND              ST77XX_BLACK

#define WIFI_COLOR_LIQUID_1               0xFE5000
#define WIFI_COLOR_LIQUID_2               0x01FFFF
#define WIFI_COLOR_LIQUID_3               0x00E784

// Startup image
const String startupImageBottle = "/BottleAperoliker.bmp";
const String startupImageGlass = "/GlassAperoliker.bmp";
const String startupImageLogo = "/LogoAperoliker.bmp";

#define TFT_TRANSPARENCY_COLOR            0x07E0
#define TFT_LOGO_POS_X                    0
#define TFT_LOGO_POS_Y                    25
#define TFT_GLASS_POS_X                   140
#define TFT_GLASS_POS_Y                   85
#define TFT_BOTTLE_POS_X                  40
#define TFT_BOTTLE_POS_Y                  5


//===============================================================
#elif defined(APEROLIC)

// Wifi Settings
#define MIXER_NAME                        "APEROLic"      // Should not exceed 15 characters and be a single word -> will be the dns name in lower case and without white spaces for example "http://aperolic.local/"
#define MIXER_PASSWORD                    "mixer1234"     // Should not exceed 15 characters

// Draw setting
#define LIQUID1_NAME                      "Aperol"        // Should not exceed 8 characters
#define LIQUID2_NAME                      "Soda"          // Should not exceed 8 characters
#define LIQUID3_NAME                      "Prosecco"      // Should not exceed 8 characters

// Mixture default setting
// Aperol: 34%, Soda: 16%, Prosecco: 50% (Official Aperol recipe)
#define LIQUID1ANGLE_DEGREES              0     //   0° -> 120° ( / 360°) = 33,33%
#define LIQUID2ANGLE_DEGREES              120   // 120° -> 177° ( / 360°) = 15,83%
#define LIQUID3ANGLE_DEGREES              177   // 177° -> 360° ( / 360°) = 50,84%

// Color defines
#define TFT_COLOR_STARTPAGE               ST77XX_ORANGE
#define TFT_COLOR_STARTPAGE_FOREGROUND    0xDF9E
#define TFT_COLOR_STARTPAGE_BACKGROUND    0xA6DC
#define TFT_COLOR_TEXT_HEADER             ST77XX_ORANGE
#define TFT_COLOR_TEXT_BODY               ST77XX_WHITE
#define TFT_COLOR_INFOBOX_BORDER          ST77XX_ORANGE
#define TFT_COLOR_INFOBOX_FOREGROUND      ST77XX_ORANGE
#define TFT_COLOR_INFOBOX_BACKGROUND      ST77XX_WHITE
#define TFT_COLOR_MENU_SELECTOR           ST77XX_ORANGE
#define TFT_COLOR_LIQUID_1                ST77XX_ORANGE
#define TFT_COLOR_LIQUID_2                0x0F1F
#define TFT_COLOR_LIQUID_3                0x0390
#define TFT_COLOR_FOREGROUND              ST77XX_WHITE
#define TFT_COLOR_BACKGROUND              ST77XX_BLACK

#define WIFI_COLOR_LIQUID_1               0xFE5000
#define WIFI_COLOR_LIQUID_2               0x01FFFF
#define WIFI_COLOR_LIQUID_3               0x00E784

// Startup image
const String startupImageBottle = "/BottleAperolic.bmp";
const String startupImageGlass = "/GlassAperolic.bmp";
const String startupImageLogo = "/LogoAperolic.bmp";

#define TFT_TRANSPARENCY_COLOR            0x07E0
#define TFT_LOGO_POS_X                    0
#define TFT_LOGO_POS_Y                    25
#define TFT_GLASS_POS_X                   140
#define TFT_GLASS_POS_Y                   85
#define TFT_BOTTLE_POS_X                  40
#define TFT_BOTTLE_POS_Y                  5


//===============================================================
#elif defined(HUGOLIKER)

// Wifi Settings
#define MIXER_NAME                        "HUGOliker"     // Should not exceed 15 characters and be a single word -> will be the dns name in lower case and without white spaces for example "http://hugoliker.local/"
#define MIXER_PASSWORD                    "mixer1234"     // Should not exceed 15 characters

// Draw setting
#define LIQUID1_NAME                      "Syrup"         // Should not exceed 8 characters
#define LIQUID2_NAME                      "Soda"          // Should not exceed 8 characters
#define LIQUID3_NAME                      "Prosecco"      // Should not exceed 8 characters

// Mixture default setting
#define LIQUID1ANGLE_DEGREES              0     //   0° -> 120° ( / 360°) = 33,33%
#define LIQUID2ANGLE_DEGREES              120   // 120° -> 177° ( / 360°) = 15,83%
#define LIQUID3ANGLE_DEGREES              177   // 177° -> 360° ( / 360°) = 50,84%

// Color defines
#define TFT_COLOR_STARTPAGE               ST77XX_YELLOW
#define TFT_COLOR_STARTPAGE_FOREGROUND    0xDF9E
#define TFT_COLOR_STARTPAGE_BACKGROUND    0xA6DC
#define TFT_COLOR_TEXT_HEADER             ST77XX_YELLOW
#define TFT_COLOR_TEXT_BODY               ST77XX_WHITE
#define TFT_COLOR_INFOBOX_BORDER          ST77XX_YELLOW
#define TFT_COLOR_INFOBOX_FOREGROUND      ST77XX_BLACK
#define TFT_COLOR_INFOBOX_BACKGROUND      ST77XX_WHITE
#define TFT_COLOR_MENU_SELECTOR           ST77XX_YELLOW
#define TFT_COLOR_LIQUID_1                ST77XX_YELLOW
#define TFT_COLOR_LIQUID_2                ST77XX_CYAN
#define TFT_COLOR_LIQUID_3                0x0390
#define TFT_COLOR_FOREGROUND              ST77XX_WHITE
#define TFT_COLOR_BACKGROUND              ST77XX_BLACK

#define WIFI_COLOR_LIQUID_1               0xD6FF00
#define WIFI_COLOR_LIQUID_2               0x01FFFF
#define WIFI_COLOR_LIQUID_3               0x00E784

// Startup image
const String startupImageBottle = "/BottleHugoliker.bmp";
const String startupImageGlass = "/GlassHugoliker.bmp";
const String startupImageLogo = "/LogoHugoliker.bmp";

#define TFT_TRANSPARENCY_COLOR            0x07E0
#define TFT_LOGO_POS_X                    0
#define TFT_LOGO_POS_Y                    25
#define TFT_GLASS_POS_X                   140
#define TFT_GLASS_POS_Y                   85
#define TFT_BOTTLE_POS_X                  40
#define TFT_BOTTLE_POS_Y                  5


//===============================================================
#elif defined(WILDBERRY)

// Wifi Settings
#define MIXER_NAME                        "WILDBerry"     // Should not exceed 15 characters and be a single word -> will be the dns name in lower case and without white spaces for example "http://wildberry.local/"
#define MIXER_PASSWORD                    "mixer1234"     // Should not exceed 15 characters

// Draw setting
#define LIQUID1_NAME                      "WildBerry"     // Should not exceed 8 characters
#define LIQUID2_NAME                      "Lillet"        // Should not exceed 8 characters
#define LIQUID3_NAME                      "Prosecco"      // Should not exceed 8 characters

// Mixture default setting
#define LIQUID1ANGLE_DEGREES              0     //   0° -> 177° ( / 360°) = 50,84% 
#define LIQUID2ANGLE_DEGREES              177   // 177° -> 240° ( / 360°) = 15,83%
#define LIQUID3ANGLE_DEGREES              240   // 240° -> 360° ( / 360°) = 33,33%

// Color defines
#define TFT_COLOR_STARTPAGE               ST77XX_MAGENTA
#define TFT_COLOR_STARTPAGE_FOREGROUND    0xDF9E
#define TFT_COLOR_STARTPAGE_BACKGROUND    0xA6DC
#define TFT_COLOR_TEXT_HEADER             ST77XX_MAGENTA
#define TFT_COLOR_TEXT_BODY               ST77XX_WHITE
#define TFT_COLOR_INFOBOX_BORDER          ST77XX_MAGENTA
#define TFT_COLOR_INFOBOX_FOREGROUND      ST77XX_MAGENTA
#define TFT_COLOR_INFOBOX_BACKGROUND      ST77XX_WHITE
#define TFT_COLOR_MENU_SELECTOR           ST77XX_MAGENTA
#define TFT_COLOR_LIQUID_1                ST77XX_MAGENTA
#define TFT_COLOR_LIQUID_2                ST77XX_YELLOW
#define TFT_COLOR_LIQUID_3                0x0390
#define TFT_COLOR_FOREGROUND              ST77XX_WHITE
#define TFT_COLOR_BACKGROUND              ST77XX_BLACK

#define WIFI_COLOR_LIQUID_1               0xBC44A1
#define WIFI_COLOR_LIQUID_2               0xD6FF00
#define WIFI_COLOR_LIQUID_3               0x00E784

// Startup image
const String startupImageBottle = "/BottleWildBerry.bmp";
const String startupImageGlass = "/GlassWildBerry.bmp";
const String startupImageLogo = "/LogoWildBerry.bmp";

#define TFT_TRANSPARENCY_COLOR            0x07E0
#define TFT_LOGO_POS_X                    0
#define TFT_LOGO_POS_Y                    25
#define TFT_GLASS_POS_X                   140
#define TFT_GLASS_POS_Y                   85
#define TFT_BOTTLE_POS_X                  40
#define TFT_BOTTLE_POS_Y                  5


//===============================================================
#else

// Wifi Settings
#define MIXER_NAME                        "Cocktailmixer"   // Should not exceed 15 characters and be a single word -> will be the dns name in lower case and without white spaces for example "http://cocktailmixer.local/"
#define MIXER_PASSWORD                    "mixer1234"       // Should not exceed 15 characters

// Draw setting
#define LIQUID1_NAME                      "Liquid 1"        // Should not exceed 8 characters
#define LIQUID2_NAME                      "Liquid 2"        // Should not exceed 8 characters
#define LIQUID3_NAME                      "Liquid 3"        // Should not exceed 8 characters

// Mixture default setting
#define LIQUID1ANGLE_DEGREES              0     //   0° -> 120° ( / 360°) = 33,33%
#define LIQUID2ANGLE_DEGREES              120   // 120° -> 240° ( / 360°) = 33,33%
#define LIQUID3ANGLE_DEGREES              240   // 240° -> 360° ( / 360°) = 33,33%

// Color defines
#define TFT_COLOR_STARTPAGE               ST77XX_RED
#define TFT_COLOR_STARTPAGE_FOREGROUND    ST77XX_BLACK
#define TFT_COLOR_STARTPAGE_BACKGROUND    ST77XX_WHITE
#define TFT_COLOR_TEXT_HEADER             ST77XX_RED
#define TFT_COLOR_TEXT_BODY               ST77XX_WHITE
#define TFT_COLOR_INFOBOX_BORDER          ST77XX_RED
#define TFT_COLOR_INFOBOX_FOREGROUND      ST77XX_RED
#define TFT_COLOR_INFOBOX_BACKGROUND      ST77XX_WHITE
#define TFT_COLOR_MENU_SELECTOR           ST77XX_RED
#define TFT_COLOR_LIQUID_1                ST77XX_RED
#define TFT_COLOR_LIQUID_2                ST77XX_GREEN
#define TFT_COLOR_LIQUID_3                ST77XX_BLUE
#define TFT_COLOR_FOREGROUND              ST77XX_WHITE
#define TFT_COLOR_BACKGROUND              ST77XX_BLACK

#define WIFI_COLOR_LIQUID_1               0xCC3D21
#define WIFI_COLOR_LIQUID_2               0x779937
#define WIFI_COLOR_LIQUID_3               0x547ACC

// Startup image
const String startupImageBottle = "/Bottle.bmp";
const String startupImageGlass = "/Glass.bmp";
const String startupImageLogo = "/Logo.bmp";

#define TFT_TRANSPARENCY_COLOR            0x07E0
#define TFT_LOGO_POS_X                    0
#define TFT_LOGO_POS_Y                    25
#define TFT_GLASS_POS_X                   140
#define TFT_GLASS_POS_Y                   85
#define TFT_BOTTLE_POS_X                  40
#define TFT_BOTTLE_POS_Y                  5

#endif

#endif
