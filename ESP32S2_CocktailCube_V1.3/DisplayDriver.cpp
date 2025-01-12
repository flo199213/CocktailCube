/**
 * Includes all display functions
 *
 * @author    Florian Staeblein
 * @date      2024/01/28
 * @copyright © 2024 Florian Staeblein
 */

//===============================================================
// Includes
//===============================================================
#include "DisplayDriver.h"

//===============================================================
// Constants
//===============================================================
static const char* TAG = "display";

//===============================================================
// Global variables
//===============================================================
 DisplayDriver Display;

//===============================================================
// Constructor
//===============================================================
DisplayDriver::DisplayDriver()
{
}

//===============================================================
// Initializes the display driver
//===============================================================
void DisplayDriver::Begin(Adafruit_ST7789* tft, bool spiffsAvailable)
{
  // Log startup info
  ESP_LOGI(TAG, "Begin initializing display driver");

  // Set display variable
  _tft = tft;

  // Initialize display
  _tft->init(TFT_WIDTH, TFT_HEIGHT, SPI_MODE3);
  _tft->invertDisplay(true);
  _tft->setRotation(3);
  _tft->setTextWrap(false);
  _tft->setFont(&FreeSans9pt7b);
  _tft->fillScreen(TFT_COLOR_BACKGROUND);

  int16_t x = TFT_WIDTH / 2;
  int16_t y = TFT_HEIGHT / 2;

  // Show starting message
  _tft->setTextColor(TFT_COLOR_FOREGROUND);
  DrawCenteredString("Booting...", x, y, false, 0);

  // Create image objects
  _imageBottle = new SPIFFSImage();
  _imageGlass = new SPIFFSImage();
  _imageLogo = new SPIFFSImage();

  // Load startup images to RAM
  if (spiffsAvailable)
  {
    // Set images available
    _imagesAvailable = (reader.LoadBMP(startupImageBottle.c_str(), _imageBottle) == IMAGE_SUCCESS &&
      reader.LoadBMP(startupImageGlass.c_str(), _imageGlass) == IMAGE_SUCCESS &&
      reader.LoadBMP(startupImageLogo.c_str(), _imageLogo) == IMAGE_SUCCESS) ? IMAGE_SUCCESS : IMAGE_ERR_FILE_NOT_FOUND;
      
    ESP_LOGI(TAG, "SPIFFS images are %s", (_imagesAvailable ? "available" : "not available"));
  }
  else
  {
    // Debug information on display
    DrawCenteredString("SPIFFS Failed", x, y + SHORTLINEOFFSET, false, 0);
    ESP_LOGE(TAG, "Loading SPIFFS images failed");
    delay(3000);
  }
  
  // Log startup info
  ESP_LOGI(TAG, "Finished initializing display driver");
}

//===============================================================
// Sets the menu state
//===============================================================
void DisplayDriver::SetMenuState(MixerState state)
{
  _menuState = state;
}

//===============================================================
// Sets the current liquid value
//===============================================================
void DisplayDriver::SetDashboardLiquid(MixtureLiquid liquid)
{
  _dashboardLiquid = liquid;
}

//===============================================================
// Sets the cleaning liquid value
//===============================================================
void DisplayDriver::SetCleaningLiquid(MixtureLiquid liquid)
{
  _cleaningLiquid = liquid;
}

//===============================================================
// Sets the angles values
//===============================================================
void DisplayDriver::SetAngles(int16_t liquid1Angle_Degrees, int16_t liquid2Angle_Degrees, int16_t liquid3Angle_Degrees)
{
  _liquid1Angle_Degrees = liquid1Angle_Degrees;
  _liquid2Angle_Degrees = liquid2Angle_Degrees;
  _liquid3Angle_Degrees = liquid3Angle_Degrees;
}

//===============================================================
// Sets the percentage values
//===============================================================
void DisplayDriver::SetPercentages(double liquid1_Percentage, double liquid2_Percentage, double liquid3_Percentage)
{
  _liquid1_Percentage = liquid1_Percentage;
  _liquid2_Percentage = liquid2_Percentage;
  _liquid3_Percentage = liquid3_Percentage;
}

//===============================================================
// Shows intro page
//===============================================================
void DisplayDriver::ShowIntroPage()
{
  // Set log
  ESP_LOGI(TAG, "Show intro page");

  // Draw intro page background
  _tft->fillRect(0, 0,                TFT_WIDTH, TFT_HEIGHT * 0.8, TFT_COLOR_STARTPAGE_BACKGROUND);
  _tft->fillRect(0, TFT_HEIGHT * 0.8, TFT_WIDTH, TFT_HEIGHT * 0.2, TFT_COLOR_STARTPAGE_FOREGROUND);

  if (_imagesAvailable == IMAGE_SUCCESS)
  {
    // Draw intro images
    _imageBottle->Draw(TFT_BOTTLE_POS_X, TFT_BOTTLE_POS_Y, _tft, TFT_TRANSPARENCY_COLOR);
    _imageGlass->Draw(TFT_GLASS_POS_X,   TFT_GLASS_POS_Y,  _tft, TFT_TRANSPARENCY_COLOR);
    _imageLogo->Draw(TFT_LOGO_POS_X,     TFT_LOGO_POS_Y,   _tft, TFT_TRANSPARENCY_COLOR);

    // Free memory
    delete _imageBottle;
    delete _imageGlass;
    //delete _imageLogo; // Do NOT delete logo image for usage with screen saver!
  }
  else
  {
    // Draw info box (fallback)
    DrawInfoBox("- Startpage -", "NO SPIFFS Files!");
  }
}

//===============================================================
// Shows help page
//===============================================================
void DisplayDriver::ShowHelpPage()
{
  // Set log
  ESP_LOGI(TAG, "Show help page");

  int16_t x = 15;
  int16_t y = HEADEROFFSET_Y + 20;

  // Clear screen
  _tft->fillScreen(TFT_COLOR_BACKGROUND);
  
  // Draw header information
  DrawHeader("Instructions", false);

  // Set text settings
  _tft->setTextSize(1);
  _tft->setTextColor(TFT_COLOR_TEXT_BODY);

  // Draw help text
  _tft->setCursor(x, y);
  _tft->print("Short Press:");
  _tft->setCursor(x, y += SHORTLINEOFFSET);
  _tft->print(" -> Change Setting");
  _tft->setCursor(x, y += SHORTLINEOFFSET);
  _tft->print("    ~ ");
  _tft->print(LIQUID1_NAME);
  _tft->setCursor(x, y += SHORTLINEOFFSET);
  _tft->print("    ~ ");
  _tft->print(LIQUID2_NAME);
  _tft->setCursor(x, y += SHORTLINEOFFSET);
  _tft->print("    ~ ");
  _tft->print(LIQUID3_NAME);
  
  _tft->setCursor(x, y += LONGLINEOFFSET);
  _tft->print("Rotate:");
  _tft->setCursor(x, y += SHORTLINEOFFSET);
  _tft->print(" -> Change Value");

  _tft->setCursor(x, y += LONGLINEOFFSET);
  _tft->print("Long Press:");
  _tft->setCursor(x, y += SHORTLINEOFFSET);  
  _tft->print(" -> Menu/Go Back");
}

//===============================================================
// Shows menu page
//===============================================================
void DisplayDriver::ShowMenuPage()
{
  // Set log
  ESP_LOGI(TAG, "Show menu page");

  // Clear screen
  _tft->fillScreen(TFT_COLOR_BACKGROUND);
  
  // Draw header information
  DrawHeader("Menu");

  // Draw menu
  DrawMenu(true);
}

//===============================================================
// Shows dashboard page
//===============================================================
void DisplayDriver::ShowDashboardPage()
{
  // Set log
  ESP_LOGI(TAG, "Show dashboard page");

  // Clear screen
  _tft->fillScreen(TFT_COLOR_BACKGROUND);
  
  // Draw header information
  DrawHeader();
  
  // Draw chart in first draw mode
  DrawDoughnutChart3();

  // Draw legend
  DrawLegend();

  // Draw current value string
  DrawCurrentValues(true);

  int16_t x0 = X0_DOUGHNUTCHART;
  int16_t y0 = TFT_HEIGHT - 30;

  // Draw enjoy message
  _tft->setTextSize(1);
  _tft->setTextColor(TFT_COLOR_FOREGROUND);
  DrawCenteredString("Enjoy it!", x0, y0, false, 0);
}

//===============================================================
// Shows cleaning page
//===============================================================
void DisplayDriver::ShowCleaningPage()
{
  // Set log
  ESP_LOGI(TAG, "Show cleaning page");

  // Clear screen
  _tft->fillScreen(TFT_COLOR_BACKGROUND);
  
  // Draw header information
  DrawHeader("Cleaning Mode");

  // Draw checkboxes
  DrawCheckBoxes();
}

//===============================================================
// Shows settings page
//===============================================================
void DisplayDriver::ShowSettingsPage()
{
  // Set log
  ESP_LOGI(TAG, "Show settings page");

  int16_t x = 15;
  int16_t y = HEADEROFFSET_Y + 25;

  // Clear screen
  _tft->fillScreen(TFT_COLOR_BACKGROUND);
  
  // Draw header information
  DrawHeader("Settings");

  double valueLiquid1 = FlowMeter.GetValueLiquid1();
  double valueLiquid2 = FlowMeter.GetValueLiquid2();
  double valueLiquid3 = FlowMeter.GetValueLiquid3();

  // Fill in settings text
  _tft->setTextSize(1);
  _tft->setTextColor(TFT_COLOR_TEXT_BODY);

  _tft->setCursor(x, y);
  _tft->print("App Version: ");
  _tft->print(APP_VERSION);

  DrawSettings(true);

  _tft->setCursor(x, y += (SHORTLINEOFFSET + 2 * LONGLINEOFFSET));
  _tft->print("Volume of liquid filled:");
  
  // Draw liquid 1 flow meter value
  _tft->setTextColor(TFT_COLOR_LIQUID_1);
  _tft->setCursor(x, y += SHORTLINEOFFSET);
  _tft->print(LIQUID1_NAME);
  _tft->print(":");
  _tft->setCursor(x + 120, y);
  _tft->print(FormatValue(valueLiquid1, 4, 2));
  _tft->print(" L");
  
  // Draw liquid 2 flow meter value
  _tft->setTextColor(TFT_COLOR_LIQUID_2);
  _tft->setCursor(x, y += SHORTLINEOFFSET);
  _tft->print(LIQUID2_NAME);
  _tft->print(":");
  _tft->setCursor(x + 120, y);
  _tft->print(FormatValue(valueLiquid2, 4, 2));
  _tft->print(" L");  
  
  // Draw liquid 3 flow meter value
  _tft->setTextColor(TFT_COLOR_LIQUID_3);
  _tft->setCursor(x, y += SHORTLINEOFFSET);
  _tft->print(LIQUID3_NAME);
  _tft->print(":");
  _tft->setCursor(x + 120, y);
  _tft->print(FormatValue(valueLiquid3, 4, 2));
  _tft->print(" L");
  
  x = 40;
  y = TFT_HEIGHT - 20;

  // Draw copyright icon
  _tft->drawXBitmap(x, y, icon_copyright, 20, 20, TFT_COLOR_TEXT_BODY);
  
  // Draw copyright text
  _tft->setCursor(x + 25, y + 15);
  _tft->setTextColor(TFT_COLOR_TEXT_BODY);
  _tft->print("2024 F.Stablein");
  _tft->drawRect(x + 105, y + 2, 2, 2, TFT_COLOR_TEXT_BODY);  // Stablein with two dots -> Stäblein
  _tft->drawRect(x + 109, y + 2, 2, 2, TFT_COLOR_TEXT_BODY);  // Stablein with two dots -> Stäblein
}

//===============================================================
// Shows screen saver page
//===============================================================
void DisplayDriver::ShowScreenSaverPage()
{
  // Set log
  ESP_LOGI(TAG, "Show screen saver page");

  // Clear screen
  _tft->fillScreen(TFT_COLOR_BACKGROUND);

  // Draw inital screen saver
  DrawScreenSaver();
}

//===============================================================
// Draws default header Text
//===============================================================
void DisplayDriver::DrawHeader()
{
  DrawHeader(String("-- ") + MIXER_NAME + " --");
}

//===============================================================
// Draws header Text
//===============================================================
void DisplayDriver::DrawHeader(const String &text, bool withIcons)
{
  int16_t x = TFT_WIDTH / 2;
  int16_t y = HEADEROFFSET_Y / 2;

  // Draw header text
  _tft->setTextSize(1);
  _tft->setTextColor(TFT_COLOR_TEXT_HEADER);
  DrawCenteredString(text, x, y, false, 0);

  x = HEADER_MARGIN;
  y = HEADEROFFSET_Y;
  int16_t x1 = TFT_WIDTH - (2 * HEADER_MARGIN);
  int16_t y1 = HEADEROFFSET_Y;

  // Draw header line
  _tft->drawLine(x, y, x1, y1, TFT_COLOR_FOREGROUND);

  if (withIcons)
  {
#if defined(WIFI_MIXER)
    // Draw wifi icons
    DrawWifiIcons(true);
#endif
  }
}

#if defined(WIFI_MIXER)
//===============================================================
// Draws the Wifi icon
//===============================================================
void DisplayDriver::DrawWifiIcons(bool isfullUpdate)
{
  int16_t x = TFT_WIDTH - 24 - 5;
  int16_t y = 2;
  int16_t width = 24;
  int16_t height = 24;

  wifi_mode_t wifiMode = Wifihandler.GetWifiMode();
  uint16_t connectedClients = Wifihandler.GetConnectedClients();

   // Check connected clients for changed value
  if (_lastDraw_ConnectedClients == connectedClients && !isfullUpdate)
  {
    return;
  }
  _lastDraw_ConnectedClients = connectedClients;

  // Clear wifi icon
  _tft->drawXBitmap(x, y, icon_wifi, width, height, TFT_COLOR_BACKGROUND);
  _tft->drawXBitmap(x, y, icon_noWifi, width, height, TFT_COLOR_BACKGROUND);

  // Draw new wifi icon
  _tft->drawXBitmap(x, y, wifiMode == WIFI_MODE_AP ? icon_wifi : icon_noWifi, width, height, TFT_COLOR_FOREGROUND);

  x = 5;
  y += 2;

  // Clear connected clients
  _tft->fillRect(x, y, width, height, TFT_COLOR_BACKGROUND);

  if (wifiMode == WIFI_MODE_AP)
  {
    // Draw new connected clients
    _tft->drawXBitmap(x, y, icon_device, width, height, TFT_COLOR_FOREGROUND);
    _tft->setCursor(x + 7, y + 17);
    _tft->setTextColor(TFT_COLOR_FOREGROUND);
    _tft->print(connectedClients);
  }
}
#endif

//===============================================================
// Draws the info box
//===============================================================
void DisplayDriver::DrawInfoBox(const String &line1, const String &line2)
{
  int16_t x = INFOBOX_MARGIN_HORI;
  int16_t y = HEADEROFFSET_Y + INFOBOX_MARGIN_VERT;
  int16_t width = TFT_WIDTH - 2 * INFOBOX_MARGIN_HORI;
  int16_t height = TFT_HEIGHT - HEADEROFFSET_Y - 2 * INFOBOX_MARGIN_VERT;

  // Draw rectangle with colored border
  _tft->fillRoundRect(x,     y,      width,     height,     INFOBOX_CORNERRADIUS, TFT_COLOR_INFOBOX_BORDER);
  _tft->fillRoundRect(x + 2, y + 2,  width - 4, height - 4, INFOBOX_CORNERRADIUS, TFT_COLOR_INFOBOX_BACKGROUND);
  
  // Move to the middle of the box
  x += width / 2;
  y += height / 2;

  // Fill in info text
  _tft->setTextSize(1);
  _tft->setTextColor(TFT_COLOR_INFOBOX_FOREGROUND);
  DrawCenteredString(line1, x, y - (SHORTLINEOFFSET / 2), false, 0);
  DrawCenteredString(line2, x, y + (SHORTLINEOFFSET / 2), false, 0);
}

//===============================================================
// Draws the menu
//===============================================================
void DisplayDriver::DrawMenu(bool isfullUpdate)
{
  int16_t x = 0;
  int16_t y = 0;
  int16_t width = 0;
  int16_t height = 0;

  int16_t marginToHeader = 40;

  if (isfullUpdate)
  {
    x = MENU_MARGIN_HORI + MENU_MARGIN_ICON;
    y = HEADEROFFSET_Y + marginToHeader - 22;
    width = 32;
    height = 32;

    // Draw icons
    _tft->drawXBitmap(x, y,                    icon_dashboard, width, height, TFT_COLOR_FOREGROUND);
    _tft->drawXBitmap(x, y += MENU_LINEOFFSET, icon_cleaning,  width, height, TFT_COLOR_FOREGROUND);
    _tft->drawXBitmap(x, y += MENU_LINEOFFSET, icon_reset,     width, height, TFT_COLOR_FOREGROUND);
    _tft->drawXBitmap(x, y += MENU_LINEOFFSET, icon_settings,  width, height, TFT_COLOR_FOREGROUND);

    x = MENU_MARGIN_HORI + MENU_MARGIN_ICON + MENU_MARGIN_TEXT;
    y = HEADEROFFSET_Y + marginToHeader;

    // Draw menu text
    _tft->setTextSize(1);
    _tft->setTextColor(TFT_COLOR_TEXT_BODY);
    _tft->setCursor(x, y);
    _tft->print("Dashboard");
    _tft->setCursor(x, y += MENU_LINEOFFSET);
    _tft->print("Cleaning Mode");
    _tft->setCursor(x, y += MENU_LINEOFFSET);
    _tft->print("Reset Mixture");
    _tft->setCursor(x, y += MENU_LINEOFFSET);
    _tft->print("Settings");
  }

  if (_lastDraw_MenuState != _menuState || isfullUpdate)
  {
    x = MENU_MARGIN_HORI - 2;
    y = HEADEROFFSET_Y + marginToHeader + ((uint16_t)_lastDraw_MenuState - 1) * MENU_LINEOFFSET - 6 - MENU_SELECTOR_HEIGHT / 2;
    width = TFT_WIDTH - 2 * MENU_MARGIN_HORI;
    height = MENU_SELECTOR_HEIGHT;

    // Reset old menu selection on display
    _tft->drawRoundRect(x, y, width, height, MENU_SELECTOR_CORNERRADIUS, TFT_COLOR_BACKGROUND);

    y = HEADEROFFSET_Y + marginToHeader + ((uint16_t)_menuState - 1) * MENU_LINEOFFSET - 6 - MENU_SELECTOR_HEIGHT / 2;

    // Draw new menu selection on display
    _tft->drawRoundRect(x, y, width, height, MENU_SELECTOR_CORNERRADIUS, TFT_COLOR_MENU_SELECTOR);

    // Save last state
    _lastDraw_MenuState = _menuState;
  }
}

//===============================================================
// Draw checkboxes
//===============================================================
void DisplayDriver::DrawCheckBoxes()
{
  int16_t x = TFT_WIDTH / 2;
  int16_t y = TFT_HEIGHT / 3;
  
  // Print selection text
  _tft->setTextColor(TFT_COLOR_FOREGROUND);
  DrawCenteredString("Select pumps for cleaning:", x, y, false, 0);

  int16_t boxWidth = 30;
  int16_t boxHeight = 30;
  int16_t boxDistance = TFT_WIDTH * 2 / 7;

  x = TFT_WIDTH / 7;
  y = HEADEROFFSET_Y + TFT_HEIGHT / 3;
  
  // Draw checkboxes
  _tft->drawRect(x,                y, boxWidth, boxHeight, TFT_COLOR_FOREGROUND);
  _tft->drawRect(x += boxDistance, y, boxWidth, boxHeight, TFT_COLOR_FOREGROUND);
  _tft->drawRect(x += boxDistance, y, boxWidth, boxHeight, TFT_COLOR_FOREGROUND);

  // Reduce rectangle for infill
  x = TFT_WIDTH / 7 + 4;
  y += 4;
  boxWidth -= 8;
  boxHeight -= 8;
  
  // Draw activated checkboxes
  _tft->fillRect(x,                y, boxWidth, boxHeight, _cleaningLiquid == eLiquidAll || _cleaningLiquid == eLiquid1 ? TFT_COLOR_STARTPAGE : TFT_COLOR_BACKGROUND);
  _tft->fillRect(x += boxDistance, y, boxWidth, boxHeight, _cleaningLiquid == eLiquidAll || _cleaningLiquid == eLiquid2 ? TFT_COLOR_STARTPAGE : TFT_COLOR_BACKGROUND);
  _tft->fillRect(x += boxDistance, y, boxWidth, boxHeight, _cleaningLiquid == eLiquidAll || _cleaningLiquid == eLiquid3 ? TFT_COLOR_STARTPAGE : TFT_COLOR_BACKGROUND);

  // Move under the boxes for liquid names
  x = TFT_WIDTH / 7 + boxWidth / 2;
  y += 2 * boxHeight;

  // Draw liquid names
  _tft->setTextColor(TFT_COLOR_LIQUID_1);
  DrawCenteredString(LIQUID1_NAME, x,                y, false, 0);
  _tft->setTextColor(TFT_COLOR_LIQUID_2);
  DrawCenteredString(LIQUID2_NAME, x += boxDistance, y, false, 0);
  _tft->setTextColor(TFT_COLOR_LIQUID_3);
  DrawCenteredString(LIQUID3_NAME, x += boxDistance, y, false, 0);
}

//===============================================================
// Draws the legend
//===============================================================
void DisplayDriver::DrawLegend()
{
  int16_t x = X_LEGEND;
  int16_t y = Y_LEGEND;
  int16_t width = WIDTH_LEGEND;
  int16_t height = HEIGHT_LEGEND;

  // Draw legend box
  _tft->drawRect(x, y, width, height, TFT_COLOR_FOREGROUND);

  int16_t marginTop = 10;
  int16_t marginBetween = 21;
  int16_t boxWidth = 30;
  int16_t boxHeight = 10;

  // Move to inner info
  x = X_LEGEND + WIDTH_LEGEND / 2 - boxWidth / 2;
  y = Y_LEGEND + marginTop;
  width = boxWidth;
  height = boxHeight;

  // Draw liquid color boxes
  _tft->fillRect(x, y,                    width, height, TFT_COLOR_LIQUID_1);
  _tft->fillRect(x, y += LOONGLINEOFFSET, width, height, TFT_COLOR_LIQUID_2);
  _tft->fillRect(x, y += LOONGLINEOFFSET, width, height, TFT_COLOR_LIQUID_3);

  // Move to inner text
  x = X_LEGEND + WIDTH_LEGEND / 2;
  y = Y_LEGEND + marginTop + marginBetween;

  // Draw liquid text
  _tft->setTextSize(1);
  _tft->setTextColor(TFT_COLOR_TEXT_BODY);  
  DrawCenteredString(LIQUID1_NAME, x, y,                    true, _dashboardLiquid == eLiquid1 ? TFT_COLOR_FOREGROUND : TFT_COLOR_BACKGROUND);
  DrawCenteredString(LIQUID2_NAME, x, y += LOONGLINEOFFSET, true, _dashboardLiquid == eLiquid2 ? TFT_COLOR_FOREGROUND : TFT_COLOR_BACKGROUND);
  DrawCenteredString(LIQUID3_NAME, x, y += LOONGLINEOFFSET, true, _dashboardLiquid == eLiquid3 ? TFT_COLOR_FOREGROUND : TFT_COLOR_BACKGROUND);
}

//===============================================================
// Draws current values
//===============================================================
void DisplayDriver::DrawCurrentValues(bool isfullUpdate)
{
  String liquid1_PercentageString = FormatValue(_liquid1_Percentage, 2, 0) + String("%");
  String liquid2_PercentageString = FormatValue(_liquid2_Percentage, 2, 0) + String("%");
  String liquid3_PercentageString = FormatValue(_liquid3_Percentage, 2, 0) + String("%");

  // Set text size
  _tft->setTextSize(1);
  
  int16_t x = 15;
  int16_t y = HEADEROFFSET_Y + 25;

    // Draw base string "Mix [100%, 100%, 100% ]"
  if (isfullUpdate)
  {
    _tft->setTextColor(TFT_COLOR_TEXT_BODY);
    _tft->setCursor(x, y);
    _tft->print("Mix [");
  }

  x += 40;
  if (_lastDraw_Liquid1String != liquid1_PercentageString || isfullUpdate)
  {
    // Reset old string on display
    _tft->setTextColor(TFT_COLOR_BACKGROUND);
    _tft->setCursor(x, y);
    _tft->print(_lastDraw_Liquid1String);
    
    // Draw new string on display
    _tft->setTextColor(TFT_COLOR_LIQUID_1);
    _tft->setCursor(x, y);
    _tft->print(liquid1_PercentageString);
    
    // Save last drawn string
    _lastDraw_Liquid1String = liquid1_PercentageString;
  }

  x += 40;
  if (isfullUpdate)
  {
    _tft->setTextColor(TFT_COLOR_TEXT_BODY);
    _tft->setCursor(x, y);
    _tft->print(",");
  }

  x += 10;
  if (_lastDraw_Liquid2String != liquid2_PercentageString || isfullUpdate)
  {
    // Reset old string on display
    _tft->setTextColor(TFT_COLOR_BACKGROUND);
    _tft->setCursor(x, y);
    _tft->print(_lastDraw_Liquid2String);
    
    // Draw new string on display
    _tft->setTextColor(TFT_COLOR_LIQUID_2);
    _tft->setCursor(x, y);
    _tft->print(liquid2_PercentageString);

    // Save last drawn string
    _lastDraw_Liquid2String = liquid2_PercentageString;
  }
  
  x += 40;
  if (isfullUpdate)
  {
    _tft->setTextColor(TFT_COLOR_TEXT_BODY);
    _tft->setCursor(x, y);
    _tft->print(",");
  }
  
  x += 10;
  if (_lastDraw_Liquid3String != liquid3_PercentageString || isfullUpdate)
  {
    // Reset old string on display
    _tft->setTextColor(TFT_COLOR_BACKGROUND);
    _tft->setCursor(x, y);
    _tft->print(_lastDraw_Liquid3String);
    
    // Draw new string on display
    _tft->setTextColor(TFT_COLOR_LIQUID_3);
    _tft->setCursor(x, y);
    _tft->print(liquid3_PercentageString);

    // Save last drawn string
    _lastDraw_Liquid3String = liquid3_PercentageString;
  }

  x += 45;
  if (isfullUpdate)
  {
    _tft->setTextColor(TFT_COLOR_TEXT_BODY);
    _tft->setCursor(x, y);
    _tft->print("]");
  }
}

//===============================================================
// Draws full doughnut chart
//===============================================================
void DisplayDriver::DrawDoughnutChart3()
{
  DrawDoughnutChart3(false, true);
}

//===============================================================
// Draws doughnut chart
//===============================================================
void DisplayDriver::DrawDoughnutChart3(bool clockwise, bool isfullUpdate)
{
  if (isfullUpdate)
  {
    // Calculate count of draw_Angle's to draw
    int16_t liquid1Distance_Degrees = GetDistanceDegrees(_liquid1Angle_Degrees, _liquid2Angle_Degrees);
    int16_t liquid2Distance_Degrees = GetDistanceDegrees(_liquid2Angle_Degrees, _liquid3Angle_Degrees);
    int16_t liquid3Distance_Degrees = GetDistanceDegrees(_liquid3Angle_Degrees, _liquid1Angle_Degrees);
  
    // Draw doughnut chart parts
    FillArc(_liquid1Angle_Degrees, liquid1Distance_Degrees, TFT_COLOR_LIQUID_1);
    FillArc(_liquid2Angle_Degrees, liquid2Distance_Degrees, TFT_COLOR_LIQUID_2);
    FillArc(_liquid3Angle_Degrees, liquid3Distance_Degrees, TFT_COLOR_LIQUID_3);
  }
  else
  {
    DrawPartial(_liquid1Angle_Degrees, _lastDraw_liquid1Angle_Degrees, TFT_COLOR_LIQUID_1, TFT_COLOR_LIQUID_3, clockwise);
    DrawPartial(_liquid2Angle_Degrees, _lastDraw_liquid2Angle_Degrees, TFT_COLOR_LIQUID_2, TFT_COLOR_LIQUID_1, clockwise);
    DrawPartial(_liquid3Angle_Degrees, _lastDraw_liquid3Angle_Degrees, TFT_COLOR_LIQUID_3, TFT_COLOR_LIQUID_2, clockwise);
  }

  // Draw black spacer and selected white
  FillArc(Move360(_liquid1Angle_Degrees, -SPACERANGLE_DEGREES), 2 * SPACERANGLE_DEGREES, _dashboardLiquid == eLiquid1 ? TFT_COLOR_FOREGROUND : TFT_COLOR_BACKGROUND);
  FillArc(Move360(_liquid2Angle_Degrees, -SPACERANGLE_DEGREES), 2 * SPACERANGLE_DEGREES, _dashboardLiquid == eLiquid2 ? TFT_COLOR_FOREGROUND : TFT_COLOR_BACKGROUND);
  FillArc(Move360(_liquid3Angle_Degrees, -SPACERANGLE_DEGREES), 2 * SPACERANGLE_DEGREES, _dashboardLiquid == eLiquid3 ? TFT_COLOR_FOREGROUND : TFT_COLOR_BACKGROUND);
  
  // Set last drawn angles
  _lastDraw_liquid1Angle_Degrees = _liquid1Angle_Degrees;
  _lastDraw_liquid2Angle_Degrees = _liquid2Angle_Degrees;
  _lastDraw_liquid3Angle_Degrees = _liquid3Angle_Degrees;
}

//===============================================================
// Only draws partial update of arcs
//===============================================================
void DisplayDriver::DrawPartial(int16_t newAngle, int16_t lastAngle,  uint16_t colorAfter, uint16_t colorBefore, bool clockwise)
{
  if (lastAngle != newAngle)
  {
    // Calculate start angle and color
    // Draw from last angle to new angle
    int16_t startAngle = Move360(lastAngle, clockwise ? -SPACERANGLE_DEGREES : SPACERANGLE_DEGREES);
    uint16_t color = clockwise ? colorBefore : colorAfter;

    // Calculate draw distance
    int16_t distance_Degrees = clockwise ? GetDistanceDegrees(lastAngle, newAngle) : -360 + GetDistanceDegrees(lastAngle, newAngle);
    
    // Draw doughnut part
    FillArc(startAngle, distance_Degrees, color);
  }
}

//===============================================================
// Draw an arc with a defined thickness
//===============================================================
void DisplayDriver::FillArc(int16_t start_angle, int16_t distance_Degrees, uint16_t color)
{
  // x,y == coords of centre of arc
  // start_angle = 0 - 359
  // distance_Degrees = absolut distance to draw in degrees
  // r_inner = inner radius
  // r_outer = outer radius
  // color = 16 bit color value

  // Draw color blocks every 1°
  int16_t drawAngle_Degrees = distance_Degrees > 0 ? 1 : - 1;
  for (int16_t i = start_angle; i != start_angle + distance_Degrees; i += drawAngle_Degrees)
  {
    // Calculate pair of coordinates for segment start
    float sx = cos((i - 90) * TFT_DEG2RAD);
    float sy = sin((i - 90) * TFT_DEG2RAD);
    int16_t x0 = sx * R_INNER_DOUGHNUTCHART + X0_DOUGHNUTCHART;
    int16_t y0 = sy * R_INNER_DOUGHNUTCHART + Y0_DOUGHNUTCHART;
    int16_t x1 = sx * R_OUTER_DOUGHNUTCHART + X0_DOUGHNUTCHART;
    int16_t y1 = sy * R_OUTER_DOUGHNUTCHART + Y0_DOUGHNUTCHART;

    // Calculate pair of coordinates for segment end
    float sx2 = cos((i + drawAngle_Degrees - 90) * TFT_DEG2RAD);
    float sy2 = sin((i + drawAngle_Degrees - 90) * TFT_DEG2RAD);
    int16_t x2 = sx2 * R_INNER_DOUGHNUTCHART + X0_DOUGHNUTCHART;
    int16_t y2 = sy2 * R_INNER_DOUGHNUTCHART + Y0_DOUGHNUTCHART;
    int16_t x3 = sx2 * R_OUTER_DOUGHNUTCHART + X0_DOUGHNUTCHART;
    int16_t y3 = sy2 * R_OUTER_DOUGHNUTCHART + Y0_DOUGHNUTCHART;

    _tft->fillTriangle(x0, y0, x1, y1, x2, y2, color);
    _tft->fillTriangle(x1, y1, x2, y2, x3, y3, color);
  }
}

//===============================================================
// Draws settings
//===============================================================
void DisplayDriver::DrawSettings(bool isfullUpdate)
{
  int16_t x = 15;
  int16_t y = HEADEROFFSET_Y + 25 + LONGLINEOFFSET;

  if (isfullUpdate)
  {
    _tft->setTextColor(TFT_COLOR_TEXT_BODY);
    _tft->setCursor(x, y);
    _tft->print("PWM CycleTime: ");
  }

  uint32_t cycleTimespan_ms = Pumps.GetCycleTimespan();

  if (_lastDraw_cycleTimespan_ms != cycleTimespan_ms || isfullUpdate)
  {
    // Clear old value
    _tft->setCursor(x + 145, y);
    _tft->setTextColor(TFT_COLOR_BACKGROUND);
    _tft->print(_lastDraw_cycleTimespan_ms);
    _tft->print(" ms");

    // Set new value
    _tft->setCursor(x + 145, y);
    _tft->setTextColor(TFT_COLOR_TEXT_BODY);
    _tft->print(cycleTimespan_ms);
    _tft->print(" ms");

    _lastDraw_cycleTimespan_ms = cycleTimespan_ms;
  }

#if defined(WIFI_MIXER)
  wifi_mode_t wifiMode = Wifihandler.GetWifiMode();

  // Move to next line
  y += SHORTLINEOFFSET;

  if (isfullUpdate)
  {
    _tft->setTextColor(TFT_COLOR_TEXT_BODY);
    _tft->setCursor(x, y);
    _tft->print("WIFI Mode: ");
  }

  if (_lastDraw_wifiMode != wifiMode || isfullUpdate)
  {
    // Clear old value
    _tft->setCursor(x + 98, y);
    _tft->setTextColor(TFT_COLOR_BACKGROUND);
    _tft->print(_lastDraw_wifiMode == WIFI_MODE_AP ? "AP" : "OFF");

    // Set new value
    _tft->setCursor(x + 98, y);
    _tft->setTextColor(TFT_COLOR_TEXT_BODY);
    _tft->print(wifiMode == WIFI_MODE_AP ? "AP" : "OFF");
    
    _lastDraw_wifiMode = wifiMode;
  }
#endif
}

//===============================================================
// Draws screen saver
//===============================================================
void DisplayDriver::DrawScreenSaver()
{
  bool hasLogo = _imagesAvailable == IMAGE_SUCCESS;
  int16_t logoWidth = hasLogo ? _imageLogo->Width() : 0;
  int16_t logoHeight = hasLogo ? _imageLogo->Height() : 0;
  
  // Move logo indexes
  int16_t logo_x = _lastLogo_x + _xDir;
  int16_t logo_y = _lastLogo_y + _yDir;

  // Move logo if image is available
  if (hasLogo)
  {
    _imageLogo->Move(_lastLogo_x, _lastLogo_y, logo_x, logo_y, _tft, TFT_COLOR_BACKGROUND, TFT_TRANSPARENCY_COLOR);
  }

  // Impact collision with the left or right edge
  if (logo_x <= -logoWidth / 2 || logo_x >= TFT_WIDTH - logoWidth / 2)
  {
    _xDir = -_xDir;
  }

  // Impact collision with the top or bottom edge
  if (logo_y <= -logoHeight / 2 || logo_y >= TFT_HEIGHT - logoHeight / 2)
  {
    _yDir = -_yDir;
  }

  // Draw stars
  for (int index = 0; index < SCREENSAVER_STARCOUNT; index++)
  {
    // Init new star, if star animation finished
    if (_stars[index].Size >= _stars[index].MaxSize)
    {
      // Clear old star only outside of the non-transparent part of the logo
      if (!hasLogo ||
        !(_stars[index].X > logo_x && _stars[index].X < logo_x + logoWidth &&
        _stars[index].Y > logo_y && _stars[index].Y < logo_y + logoHeight &&
        _imageLogo->GetPixel(_stars[index].X - logo_x, _stars[index].Y - logo_y) != TFT_TRANSPARENCY_COLOR))
      {
        DrawStar(_stars[index].X, _stars[index].Y, _stars[index].FullStars, TFT_COLOR_BACKGROUND, _stars[index].Size);
      }

      _stars[index].X = random(0, TFT_WIDTH);
      _stars[index].Y = random(0, TFT_HEIGHT);
      _stars[index].MaxSize = random(1, 6);
      _stars[index].FullStars = random(0, 12) < 6 ? true : false;
      _stars[index].Size = 0;
    }

    // Draw new star only outside of the non-transparent part of the logo
    if (!hasLogo || 
      !(_stars[index].X > logo_x && _stars[index].X < logo_x + logoWidth &&
      _stars[index].Y > logo_y && _stars[index].Y < logo_y + logoHeight &&
      _imageLogo->GetPixel(_stars[index].X - logo_x, _stars[index].Y - logo_y) != TFT_TRANSPARENCY_COLOR))
    {
      DrawStar(_stars[index].X, _stars[index].Y, _stars[index].FullStars, TFT_COLOR_FOREGROUND, _stars[index].Size);
    }

    // Increment star size
    _stars[index].Size++;
  }

  _lastLogo_x = logo_x;
  _lastLogo_y = logo_y;
}

//===============================================================
// Draws a star
//===============================================================
void DisplayDriver::DrawStar(int16_t x0, int16_t y0, bool fullStars, uint16_t color, int16_t size)
{
  _tft->writePixel(x0, y0, color);

  if (size > 0)
  {
    DrawStarTail(x0, y0, 1, 2, fullStars, color);
  }
  if (size > 1)
  {
    DrawStarTail(x0, y0, 4, 5, fullStars, color);
  }
  if (size > 2)
  {
    DrawStarTail(x0, y0, 7, 8, fullStars, color);
  }
  if (size > 3)
  {
    DrawStarTail(x0, y0, 10, 11, fullStars, color);
  }
  if (size > 4)
  {
    DrawStarTail(x0, y0, 13, 14, fullStars, color);
  }
}

//===============================================================
// Draws a star tail
//===============================================================
void DisplayDriver::DrawStarTail(int16_t x0, int16_t y0, int16_t start, int16_t end, bool fullStars, uint16_t color)
{
  // Nach oben
  _tft->writeLine(x0, y0 - start, x0, y0 - end, color);

  // Nach unten
  _tft->writeLine(x0, y0 + start, x0, y0 + end, color);

  // Nach rechts
  _tft->writeLine(x0 + start, y0, x0 + end, y0, color);

  // Nach links
  _tft->writeLine(x0 - start, y0, x0 - end, y0, color);

  if (fullStars)
  {
    // Nach rechts oben
    _tft->writeLine(x0 + start, y0 - start, x0 + end, y0 - end, color);

    // Nach links oben
    _tft->writeLine(x0 - start, y0 - start, x0 - end, y0 - end, color);

    // Nach rechts unten
    _tft->writeLine(x0 + start, y0 + start, x0 + end, y0 + end, color);

    // Nach links unten
    _tft->writeLine(x0 - start, y0 + start, x0 - end, y0 + end, color);
  }
}

//===============================================================
// Draws a string centered
//===============================================================
void DisplayDriver::DrawCenteredString(const String &text, int16_t x, int16_t y, bool underlined, uint16_t lineColor)
{
  // Get text bounds
  int16_t x1, y1;
  uint16_t w, h;
  _tft->getTextBounds(text, x, y, &x1, &y1, &w, &h);

  // Calculate cursor position
  int16_t x_text = x - w / 2;
  int16_t y_text = y + h / 2;
  _tft->setCursor(x_text, y_text);
  
  // Print text
  _tft->print(text);

  // Underline if active
  if (underlined)
  {
    _tft->drawLine(x_text, y + h, x_text + w, y + h, lineColor);
  }
}

//===============================================================
// Formats double value
//===============================================================
String DisplayDriver::FormatValue(double value, int mainPlaces, int decimalPlaces)
{
  String returnValue = "";

  dtostrf(value, mainPlaces, decimalPlaces, _output);
  returnValue += String(_output);

  return returnValue;
}
