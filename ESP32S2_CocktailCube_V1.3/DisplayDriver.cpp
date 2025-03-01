/*
 * Includes all display functions
 *
 * @author    Florian Stäblein
 * @date      2025/01/01
 * @copyright © 2025 Florian Stäblein
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
void DisplayDriver::Begin(Adafruit_ST7789* tft)
{
  // Log startup info
  ESP_LOGI(TAG, "Begin initializing display driver");

  // Set display variable
  _tft = tft;

  // Set speed to 40MHz
  _tft->setSPISpeed(40000000);

  // Initialize display
  _tft->init(TFT_WIDTH, TFT_HEIGHT, SPI_MODE3);
  _tft->invertDisplay(true);
  _tft->setRotation(3);
  _tft->setTextWrap(false);
  _tft->setFont(&FreeSans9pt7b);
  _tft->fillScreen(ST77XX_BLACK);

  int16_t x = TFT_WIDTH / 2;
  int16_t y = TFT_HEIGHT / 2;

  // Show starting message
  _tft->setTextColor(ST77XX_WHITE);
  DrawCenteredString("Booting...", x, y);

  // Log startup info
  ESP_LOGI(TAG, "Finished initializing display driver");
}

//===============================================================
// Loads the images from spiffs
//===============================================================
void DisplayDriver::LoadImages()
{
  // Log info
  ESP_LOGI(TAG, "Begin loading images");

  // Load mandatory images
  LoadImage(&_imageLogo, Config.imageLogo);
  LoadImage(&_imageGlass, Config.imageGlass);
  LoadImage(&_imageBottle1, Config.imageBottle1);

  // Load non-mixer images
  if (!Config.isMixer)
  {
    LoadImage(&_imageBottle2, Config.imageBottle2);
    LoadImage(&_imageBottle3, Config.imageBottle3);
    LoadImage(&_imageBottle4, Config.imageBottle4);
  }

  // Log info
  ESP_LOGI(TAG, "Finished loading images");
}

//===============================================================
// Loads an image and checks for deallocation before loading
//===============================================================
void DisplayDriver::LoadImage(SPIFFSBMPImage* _image, String fileName)
{
  // Always delete before loading
  _image->Deallocate();
  
  // Load image to RAM
  ImageReturnCode returnCode = _image->Allocate(fileName);
  ESP_LOGI(TAG, "Load image '%s': %s (Heap: %d / %d Bytes)", fileName.c_str(), _image->PrintStatus(returnCode).c_str(), ESP.getFreeHeap(), ESP.getHeapSize());
}

//===============================================================
// Shows intro page
//===============================================================
void DisplayDriver::ShowIntroPage()
{
  // Set log
  ESP_LOGI(TAG, "Show intro page");

  // Draw intro page background
  _tft->fillRect(0, 0,                TFT_WIDTH, TFT_HEIGHT * 0.8, Config.tftColorStartPageBackground);
  _tft->fillRect(0, TFT_HEIGHT * 0.8, TFT_WIDTH, TFT_HEIGHT * 0.2, Config.tftColorStartPageForeground);

  // If not a single image is available, show info message
  if (!_imageBottle1.IsValid() &&
    !_imageGlass.IsValid() &&
    !_imageLogo.IsValid())
  {
    // Draw info box (fallback)
    DrawInfoBox("- Startpage -", "No Image Files!");
  }
  else
  {
    // Draw intro images (Attention: Order is decisive!)
    _imageBottle1.Draw(Config.tftBottlePosX, Config.tftBottlePosY, _tft, 0, false, true);
    _imageGlass.Draw(Config.tftGlassPosX, Config.tftGlassPosY, _tft, 0, false, true);
    _imageLogo.Draw(Config.tftLogoPosX, Config.tftLogoPosY, _tft, 0, false, true);
  }

  // Do NOT delete logo image (Usage for screensaver!)
  //_imageLogo->Deallocate();
  
  // Always delete glass image
  _imageGlass.Deallocate();
  
  // Only delete bottle images if mixer (Otherwise usage for bar display!)
  if (Config.isMixer)
  {
    _imageBottle1.Deallocate();
    _imageBottle2.Deallocate();
    _imageBottle3.Deallocate();
    _imageBottle4.Deallocate();
  }

  ESP_LOGI(TAG, "After intro page (images free-ed):");
  ESP_LOGI(TAG, "HeapSize : %d", ESP.getHeapSize());
  ESP_LOGI(TAG, "HeapFree : %d", ESP.getFreeHeap());
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
  _tft->fillScreen(Config.tftColorBackground);
  
  // Draw header information
  DrawHeader("Instructions", false);

  // Set text settings
  _tft->setTextSize(1);
  _tft->setTextColor(Config.tftColorTextBody);

  // Draw help text
  _tft->setCursor(x, y);
  _tft->print("Short Press:");
  _tft->setCursor(x, y += SHORTLINEOFFSET);
  _tft->print(" -> Change Setting");
  _tft->setCursor(x, y += SHORTLINEOFFSET);
  _tft->print("    ~ ");
  _tft->print(Config.liquidName1);
  _tft->setCursor(x, y += SHORTLINEOFFSET);
  _tft->print("    ~ ");
  _tft->print(Config.liquidName2);
  _tft->setCursor(x, y += SHORTLINEOFFSET);
  _tft->print("    ~ ");
  _tft->print(Config.liquidName3);
  
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
  _tft->fillScreen(Config.tftColorBackground);
  
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
  _tft->fillScreen(Config.tftColorBackground);
  
  // Draw header information
  DrawHeader();
  
  if (Config.isMixer)
  {
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
    _tft->setTextColor(Config.tftColorForeground);
    DrawCenteredString("Enjoy it!", x0, y0);
  }
  else
  {
    // Draw bar
    Display.DrawBar(true, true);
  }
}

//===============================================================
// Shows cleaning page
//===============================================================
void DisplayDriver::ShowCleaningPage()
{
  // Set log
  ESP_LOGI(TAG, "Show cleaning page");

  // Clear screen
  _tft->fillScreen(Config.tftColorBackground);
  
  // Draw header information
  DrawHeader("Cleaning Mode");

  int16_t x = TFT_WIDTH / 2;
  int16_t y = TFT_HEIGHT / 3;
  
  // Print selection text
  _tft->setTextColor(Config.tftColorForeground);
  DrawCenteredString("Select pumps for cleaning:", x, y);

  MixtureLiquid cleaningLiquid = Statemachine.GetCleaningLiquid();

  // Draw checkboxes
  DrawCheckBoxes(cleaningLiquid);
}

//===============================================================
// Shows bar page
//===============================================================
void DisplayDriver::ShowBarPage()
{
  // Clear screen
  _tft->fillScreen(Config.tftColorBackground);
  
  // Draw header information
  DrawHeader("Bar Stock");
  
  // Draw bar
  Display.DrawBar(false, true);
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
  _tft->fillScreen(Config.tftColorBackground);
  
  // Draw header information
  DrawHeader("Settings");

  double valueLiquid1 = FlowMeter.GetValueLiquid1();
  double valueLiquid2 = FlowMeter.GetValueLiquid2();
  double valueLiquid3 = FlowMeter.GetValueLiquid3();

  // Fill in settings text
  _tft->setTextSize(1);
  _tft->setTextColor(Config.tftColorTextBody);

  _tft->setCursor(x, y);
  _tft->print("App Version: ");
  _tft->print(APP_VERSION);

  DrawSettings(true);

  _tft->setCursor(x, y += (SHORTLINEOFFSET + 2 * LONGLINEOFFSET + SHORTLINEOFFSET) - 4);
  _tft->print("Volume of liquid filled:");
  
  // Draw liquid 1 flow meter value
  _tft->setTextColor(Config.tftColorLiquid1);
  _tft->setCursor(x, y += SHORTLINEOFFSET);
  _tft->print(Config.liquidName1);
  _tft->print(":");
  _tft->setCursor(x + 120, y);
  _tft->print(FormatValue(valueLiquid1, 4, 2));
  _tft->print(" L");
  
  // Draw liquid 2 flow meter value
  _tft->setTextColor(Config.tftColorLiquid2);
  _tft->setCursor(x, y += SHORTLINEOFFSET);
  _tft->print(Config.liquidName2);
  _tft->print(":");
  _tft->setCursor(x + 120, y);
  _tft->print(FormatValue(valueLiquid2, 4, 2));
  _tft->print(" L");  
  
  // Draw liquid 3 flow meter value
  _tft->setTextColor(Config.tftColorLiquid3);
  _tft->setCursor(x, y += SHORTLINEOFFSET);
  _tft->print(Config.liquidName3);
  _tft->print(":");
  _tft->setCursor(x + 120, y);
  _tft->print(FormatValue(valueLiquid3, 4, 2));
  _tft->print(" L");
  
  x = 40;
  y = TFT_HEIGHT - 20;

  // Draw copyright icon
  _tft->drawXBitmap(x, y, icon_copyright, 20, 20, Config.tftColorTextBody);
  
  // Draw copyright text
  _tft->setCursor(x + 25, y + 15);
  _tft->setTextColor(Config.tftColorTextBody);
  _tft->print(APP_COPYRIGHT);
  _tft->print(" F.Stablein");
  _tft->drawRect(x + 105, y + 2, 2, 2, Config.tftColorTextBody);  // Stablein with two dots -> Stäblein
  _tft->drawRect(x + 109, y + 2, 2, 2, Config.tftColorTextBody);  // Stablein with two dots -> Stäblein
}

//===============================================================
// Shows screen saver page
//===============================================================
void DisplayDriver::ShowScreenSaverPage()
{
  // Set log
  ESP_LOGI(TAG, "Show screen saver page");

  // Clear screen
  _tft->fillScreen(Config.tftColorBackground);

  // Draw inital screen saver
  DrawScreenSaver();
}

//===============================================================
// Draws default header Text
//===============================================================
void DisplayDriver::DrawHeader()
{
  DrawHeader(String("-- ") + Config.mixerName + " --");
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
  _tft->setTextColor(Config.tftColorTextHeader);
  DrawCenteredString(text, x, y);

  x = HEADER_MARGIN;
  y = HEADEROFFSET_Y;
  int16_t x1 = TFT_WIDTH - (2 * HEADER_MARGIN);
  int16_t y1 = HEADEROFFSET_Y;

  // Draw header line
  _tft->drawLine(x, y, x1, y1, Config.tftColorForeground);

  if (withIcons)
  {
    // Draw wifi icons
    DrawWifiIcons(true);
  }
}

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
  _tft->drawXBitmap(x, y, icon_wifi, width, height, Config.tftColorBackground);
  _tft->drawXBitmap(x, y, icon_noWifi, width, height, Config.tftColorBackground);

  // Draw new wifi icon
  _tft->drawXBitmap(x, y, wifiMode == WIFI_MODE_AP ? icon_wifi : icon_noWifi, width, height, Config.tftColorForeground);

  x = 5;
  y += 2;

  // Clear connected clients
  _tft->fillRect(x, y, width, height, Config.tftColorBackground);

  if (wifiMode == WIFI_MODE_AP)
  {
    // Draw new connected clients
    _tft->drawXBitmap(x, y, icon_device, width, height, Config.tftColorForeground);
    _tft->setCursor(x + 7, y + 17);
    _tft->setTextColor(Config.tftColorForeground);
    _tft->print(connectedClients);
  }
}

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
  _tft->fillRoundRect(x,     y,      width,     height,     INFOBOX_CORNERRADIUS, Config.tftColorInfoBoxBorder);
  _tft->fillRoundRect(x + 2, y + 2,  width - 4, height - 4, INFOBOX_CORNERRADIUS, Config.tftColorInfoBoxBackground);
  
  // Move to the middle of the box
  x += width / 2;
  y += height / 2;

  // Fill in info text
  _tft->setTextSize(1);
  _tft->setTextColor(Config.tftColorInfoBoxForeground);
  DrawCenteredString(line1, x, y - (SHORTLINEOFFSET / 2));
  DrawCenteredString(line2, x, y + (SHORTLINEOFFSET / 2));
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
    _tft->drawXBitmap(x, y,                    icon_dashboard, width, height, Config.tftColorForeground);
    _tft->drawXBitmap(x, y += MENU_LINEOFFSET, icon_cleaning,  width, height, Config.tftColorForeground);
    _tft->drawXBitmap(x, y += MENU_LINEOFFSET, icon_reset,     width, height, Config.tftColorForeground);
    _tft->drawXBitmap(x, y += MENU_LINEOFFSET, icon_settings,  width, height, Config.tftColorForeground);

    x = MENU_MARGIN_HORI + MENU_MARGIN_ICON + MENU_MARGIN_TEXT;
    y = HEADEROFFSET_Y + marginToHeader;

    // Draw menu text
    _tft->setTextSize(1);
    _tft->setTextColor(Config.tftColorTextBody);
    _tft->setCursor(x, y);
    _tft->print("Dashboard");
    _tft->setCursor(x, y += MENU_LINEOFFSET);
    _tft->print("Cleaning Mode");
    _tft->setCursor(x, y += MENU_LINEOFFSET);
    if (Config.isMixer)
    {
      _tft->print("Reset Mixture");
    }
    else
    {
      _tft->print("Bar Stock");
    }
    _tft->setCursor(x, y += MENU_LINEOFFSET);
    _tft->print("Settings");
  }

  MixerState menuState = Statemachine.GetMenuState();

  if (_lastDraw_MenuState != menuState || isfullUpdate)
  {
    uint16_t offsetIndex = (uint16_t)_lastDraw_MenuState - 1;
    if (!Config.isMixer &&
      _lastDraw_MenuState == eBar)
    {
      // Fix for bar stock (enum index 6)
      offsetIndex = 2;
    }

    x = MENU_MARGIN_HORI - 2;
    y = HEADEROFFSET_Y + marginToHeader + offsetIndex * MENU_LINEOFFSET - 6 - MENU_SELECTOR_HEIGHT / 2;
    width = TFT_WIDTH - 2 * MENU_MARGIN_HORI;
    height = MENU_SELECTOR_HEIGHT;

    // Reset old menu selection on display
    _tft->drawRoundRect(x, y, width, height, MENU_SELECTOR_CORNERRADIUS, Config.tftColorBackground);

    offsetIndex = (uint16_t)menuState - 1;
    if (!Config.isMixer &&
      menuState == eBar)
    {
      // Fix for bar stock (enum index 6)
      offsetIndex = 2;
    }

    y = HEADEROFFSET_Y + marginToHeader + offsetIndex * MENU_LINEOFFSET - 6 - MENU_SELECTOR_HEIGHT / 2;

    // Draw new menu selection on display
    _tft->drawRoundRect(x, y, width, height, MENU_SELECTOR_CORNERRADIUS, Config.tftColorMenuSelector);

    // Save last state
    _lastDraw_MenuState = menuState;
  }
}

//===============================================================
// Draw bar
//===============================================================
void DisplayDriver::DrawBar(bool isDashboard, bool isfullUpdate)
{
  int16_t spacing = 78;
  int16_t x0 = TFT_WIDTH / 2; // Mid screen
  int16_t y = HEADEROFFSET_Y + 10;

  MixtureLiquid dashboardLiquid = Statemachine.GetDashboardLiquid();
  BarBottle barBottle1 = Statemachine.GetBarBottle(0);
  BarBottle barBottle2 = Statemachine.GetBarBottle(1);
  BarBottle barBottle3 = Statemachine.GetBarBottle(2);

  double liquidPercentage1 = Statemachine.GetBarPercentage(eLiquid1);
  double liquidPercentage2 = Statemachine.GetBarPercentage(eLiquid2);
  double liquidPercentage3 = Statemachine.GetBarPercentage(eLiquid3);

  // Draw only check boxes if complete bar stock is empty
  if (isDashboard &&
    barBottle1 == eEmpty &&
    barBottle2 == eEmpty &&
    barBottle3 == eEmpty)
  {
    // Print selection text
    _tft->setTextColor(Config.tftColorForeground);
    DrawCenteredString("Select WINE for dispensing:", x0, y + 25, false, 0, true, 0x528A); // Gray

    // Draw checkboxes
    DrawCheckBoxes(dashboardLiquid);
  }
  else
  {
    // Draw each bottle
    DrawBarPart(x0 - spacing, y, eLiquid1, barBottle1, _lastDraw_barBottle1, liquidPercentage1, _lastDraw_liquidPercentage1, Config.liquidName1, Config.tftColorLiquid1, isDashboard, isfullUpdate);
    DrawBarPart(x0,           y, eLiquid2, barBottle2, _lastDraw_barBottle2, liquidPercentage2, _lastDraw_liquidPercentage2, Config.liquidName2, Config.tftColorLiquid2, isDashboard, isfullUpdate);
    DrawBarPart(x0 + spacing, y, eLiquid3, barBottle3, _lastDraw_barBottle3, liquidPercentage3, _lastDraw_liquidPercentage3, Config.liquidName3, Config.tftColorLiquid3, isDashboard, isfullUpdate);

    if (isDashboard &&
      (isfullUpdate || dashboardLiquid != _lastDraw_SelectedLiquid))
    {
      // Print selection text
      _tft->setTextColor(Config.tftColorForeground);
      DrawCenteredString("Select WINE for dispensing:", x0, y + 25, false, 0, true, 0x528A); // Gray
    }

    _lastDraw_SelectedLiquid = dashboardLiquid;
    _lastDraw_barBottle1 = barBottle1;
    _lastDraw_barBottle2 = barBottle2;
    _lastDraw_barBottle3 = barBottle3;
    _lastDraw_liquidPercentage1 = liquidPercentage1;
    _lastDraw_liquidPercentage2 = liquidPercentage2;
    _lastDraw_liquidPercentage3 = liquidPercentage3;
  }

  ESP_LOGI(TAG, "_barBottles: %d %d %d", barBottle1, barBottle2, barBottle3);
}

//===============================================================
// Draw checkboxes
//===============================================================
void DisplayDriver::DrawCheckBoxes(MixtureLiquid liquid)
{
  int16_t boxSize = 30;
  int16_t spacing = 78;
  int16_t x0 = TFT_WIDTH / 2; // Mid screen
  int16_t y = HEADEROFFSET_Y + 80;
  
  // Draw checkboxes
  _tft->drawRect(x0 - boxSize / 2 - spacing, y, boxSize, boxSize, Config.tftColorForeground);
  _tft->drawRect(x0 - boxSize / 2,           y, boxSize, boxSize, Config.tftColorForeground);
  _tft->drawRect(x0 - boxSize / 2 + spacing, y, boxSize, boxSize, Config.tftColorForeground);
  
  // Draw activated checkboxes
  _tft->fillRect(x0 - boxSize / 2 + 4 - spacing, y + 4, boxSize - 8, boxSize - 8, liquid == eLiquidAll || liquid == eLiquid1 ? Config.tftColorStartPage : Config.tftColorBackground);
  _tft->fillRect(x0 - boxSize / 2 + 4,           y + 4, boxSize - 8, boxSize - 8, liquid == eLiquidAll || liquid == eLiquid2 ? Config.tftColorStartPage : Config.tftColorBackground);
  _tft->fillRect(x0 - boxSize / 2 + 4 + spacing, y + 4, boxSize - 8, boxSize - 8, liquid == eLiquidAll || liquid == eLiquid3 ? Config.tftColorStartPage : Config.tftColorBackground);

  // Move under the boxes for liquid names
  y = HEADEROFFSET_Y + 140;

  // Draw liquid names
  _tft->setTextColor(Config.tftColorLiquid1);
  DrawCenteredString(Config.liquidName1, x0 - spacing, y);
  _tft->setTextColor(Config.tftColorLiquid2);
  DrawCenteredString(Config.liquidName2, x0, y);
  _tft->setTextColor(Config.tftColorLiquid3);
  DrawCenteredString(Config.liquidName3, x0 + spacing, y);
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
  _tft->drawRect(x, y, width, height, Config.tftColorForeground);

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
  _tft->fillRect(x, y,                    width, height, Config.tftColorLiquid1);
  _tft->fillRect(x, y += LOONGLINEOFFSET, width, height, Config.tftColorLiquid2);
  _tft->fillRect(x, y += LOONGLINEOFFSET, width, height, Config.tftColorLiquid3);

  // Move to inner text
  x = X_LEGEND + WIDTH_LEGEND / 2;
  y = Y_LEGEND + marginTop + marginBetween;

  MixtureLiquid dashboardLiquid = Statemachine.GetDashboardLiquid();

  // Draw liquid text
  _tft->setTextSize(1);
  _tft->setTextColor(Config.tftColorTextBody);  
  DrawCenteredString(Config.liquidName1, x, y,                    true, dashboardLiquid == eLiquid1 ? Config.tftColorForeground : Config.tftColorBackground, false, 0);
  DrawCenteredString(Config.liquidName2, x, y += LOONGLINEOFFSET, true, dashboardLiquid == eLiquid2 ? Config.tftColorForeground : Config.tftColorBackground, false, 0);
  DrawCenteredString(Config.liquidName3, x, y += LOONGLINEOFFSET, true, dashboardLiquid == eLiquid3 ? Config.tftColorForeground : Config.tftColorBackground, false, 0);
}

//===============================================================
// Draws current values
//===============================================================
void DisplayDriver::DrawCurrentValues(bool isfullUpdate)
{
  double liquidPercentage1 = Statemachine.GetPumpPercentage(eLiquid1);
  double liquidPercentage2 = Statemachine.GetPumpPercentage(eLiquid2);
  double liquidPercentage3 = Statemachine.GetPumpPercentage(eLiquid3);

  String liquidPercentage1_String = FormatValue(liquidPercentage1, 2, 0) + String("%");
  String liquidPercentage2_String = FormatValue(liquidPercentage2, 2, 0) + String("%");
  String liquidPercentage3_String = FormatValue(liquidPercentage3, 2, 0) + String("%");

  // Set text size
  _tft->setTextSize(1);
  
  int16_t x = 15;
  int16_t y = HEADEROFFSET_Y + 25;

    // Draw base string "Mix [100%, 100%, 100% ]"
  if (isfullUpdate)
  {
    _tft->setTextColor(Config.tftColorTextBody);
    _tft->setCursor(x, y);
    _tft->print("Mix [");
  }

  x += 40;
  if (_lastDraw_liquidPercentage1_String != liquidPercentage1_String || isfullUpdate)
  {
    // Reset old string on display
    _tft->setTextColor(Config.tftColorBackground);
    _tft->setCursor(x, y);
    _tft->print(_lastDraw_liquidPercentage1_String);
    
    // Draw new string on display
    _tft->setTextColor(Config.tftColorLiquid1);
    _tft->setCursor(x, y);
    _tft->print(liquidPercentage1_String);
    
    // Save last drawn string
    _lastDraw_liquidPercentage1_String = liquidPercentage1_String;
  }

  x += 40;
  if (isfullUpdate)
  {
    _tft->setTextColor(Config.tftColorTextBody);
    _tft->setCursor(x, y);
    _tft->print(",");
  }

  x += 10;
  if (_lastDraw_liquidPercentage2_String != liquidPercentage2_String || isfullUpdate)
  {
    // Reset old string on display
    _tft->setTextColor(Config.tftColorBackground);
    _tft->setCursor(x, y);
    _tft->print(_lastDraw_liquidPercentage2_String);
    
    // Draw new string on display
    _tft->setTextColor(Config.tftColorLiquid2);
    _tft->setCursor(x, y);
    _tft->print(liquidPercentage2_String);

    // Save last drawn string
    _lastDraw_liquidPercentage2_String = liquidPercentage2_String;
  }
  
  x += 40;
  if (isfullUpdate)
  {
    _tft->setTextColor(Config.tftColorTextBody);
    _tft->setCursor(x, y);
    _tft->print(",");
  }
  
  x += 10;
  if (_lastDraw_liquidPercentage3_String != liquidPercentage3_String || isfullUpdate)
  {
    // Reset old string on display
    _tft->setTextColor(Config.tftColorBackground);
    _tft->setCursor(x, y);
    _tft->print(_lastDraw_liquidPercentage3_String);
    
    // Draw new string on display
    _tft->setTextColor(Config.tftColorLiquid3);
    _tft->setCursor(x, y);
    _tft->print(liquidPercentage3_String);

    // Save last drawn string
    _lastDraw_liquidPercentage3_String = liquidPercentage3_String;
  }

  x += 45;
  if (isfullUpdate)
  {
    _tft->setTextColor(Config.tftColorTextBody);
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
  MixtureLiquid dashboardLiquid = Statemachine.GetDashboardLiquid();
  int16_t liquidAngle1 = Statemachine.GetAngle(eLiquid1);
  int16_t liquidAngle2 = Statemachine.GetAngle(eLiquid2);
  int16_t liquidAngle3 = Statemachine.GetAngle(eLiquid3);

  if (isfullUpdate)
  {
    // Calculate count of draw_Angle's to draw
    int16_t liquid1Distance_Degrees = GetDistanceDegrees(liquidAngle1, liquidAngle2);
    int16_t liquid2Distance_Degrees = GetDistanceDegrees(liquidAngle2, liquidAngle3);
    int16_t liquid3Distance_Degrees = GetDistanceDegrees(liquidAngle3, liquidAngle1);
  
    // Draw doughnut chart parts
    FillArc(liquidAngle1, liquid1Distance_Degrees, Config.tftColorLiquid1);
    FillArc(liquidAngle2, liquid2Distance_Degrees, Config.tftColorLiquid2);
    FillArc(liquidAngle3, liquid3Distance_Degrees, Config.tftColorLiquid3);
  }
  else
  {
    DrawPartial(liquidAngle1, _lastDraw_liquidAngle1, Config.tftColorLiquid1, Config.tftColorLiquid3, clockwise);
    DrawPartial(liquidAngle2, _lastDraw_liquidAngle2, Config.tftColorLiquid2, Config.tftColorLiquid1, clockwise);
    DrawPartial(liquidAngle3, _lastDraw_liquidAngle3, Config.tftColorLiquid3, Config.tftColorLiquid2, clockwise);
  }

  // Draw black spacer and selected white
  FillArc(Move360(liquidAngle1, -SPACERANGLE_DEGREES), 2 * SPACERANGLE_DEGREES, dashboardLiquid == eLiquid1 ? Config.tftColorForeground : Config.tftColorBackground);
  FillArc(Move360(liquidAngle2, -SPACERANGLE_DEGREES), 2 * SPACERANGLE_DEGREES, dashboardLiquid == eLiquid2 ? Config.tftColorForeground : Config.tftColorBackground);
  FillArc(Move360(liquidAngle3, -SPACERANGLE_DEGREES), 2 * SPACERANGLE_DEGREES, dashboardLiquid == eLiquid3 ? Config.tftColorForeground : Config.tftColorBackground);
  
  // Set last drawn angles
  _lastDraw_liquidAngle1 = liquidAngle1;
  _lastDraw_liquidAngle2 = liquidAngle2;
  _lastDraw_liquidAngle3 = liquidAngle3;
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
  int16_t y = HEADEROFFSET_Y + 25 + LONGLINEOFFSET - 2;

  // Clear settings text
  DrawSettingsEntry(x, y + 0 * SHORTLINEOFFSET, _lastDraw_previousSettingName, _lastDraw_previousSettingValue, false, true, isfullUpdate);
  DrawSettingsEntry(x, y + 1 * SHORTLINEOFFSET, _lastDraw_currentSettingName, _lastDraw_currentSettingValue, _lastDraw_settingSelected, true, isfullUpdate);
  DrawSettingsEntry(x, y + 2 * SHORTLINEOFFSET, _lastDraw_nextSettingName, _lastDraw_nextSettingValue, false, true, isfullUpdate);

  // Get previous, current and next settings
  MixerSetting currentSetting = Statemachine.GetMixerSetting();
  MixerSetting previousSetting = (uint16_t)currentSetting - 1 < 0 ? (MixerSetting)(MixerSettingMax - 1) : (MixerSetting)(currentSetting - 1);
  MixerSetting nextSetting = (uint16_t)currentSetting + 1 >= MixerSettingMax ? ePWM : (MixerSetting)(currentSetting + 1);

  // Update names
  _lastDraw_previousSettingName = GetSettingsName(previousSetting);
  _lastDraw_currentSettingName = GetSettingsName(currentSetting);
  _lastDraw_nextSettingName = GetSettingsName(nextSetting);

  // Update values
  _lastDraw_previousSettingValue = GetSettingsValue(previousSetting);
  _lastDraw_currentSettingValue = GetSettingsValue(currentSetting);
  _lastDraw_nextSettingValue = GetSettingsValue(nextSetting);

  // Update selected
  _lastDraw_settingSelected = Statemachine.GetSettingSelected();

  // Draw new settings text
  DrawSettingsEntry(x, y + 0 * SHORTLINEOFFSET, _lastDraw_previousSettingName, _lastDraw_previousSettingValue, false, false, isfullUpdate);
  DrawSettingsEntry(x, y + 1 * SHORTLINEOFFSET, _lastDraw_currentSettingName, _lastDraw_currentSettingValue, _lastDraw_settingSelected, false, isfullUpdate);
  DrawSettingsEntry(x, y + 2 * SHORTLINEOFFSET, _lastDraw_nextSettingName, _lastDraw_nextSettingValue, false, false, isfullUpdate);

  // Draw Cursor
  if (isfullUpdate)
  {
    _tft->setTextColor(Config.tftColorTextHeader);
    _tft->setCursor(0, y - 2 + SHORTLINEOFFSET);
    _tft->print("->");
  }
}

//===============================================================
// Draws screen saver
//===============================================================
void DisplayDriver::DrawScreenSaver()
{
  int16_t logoWidth = _imageLogo.IsValid() ? _imageLogo.Width() : 0;
  int16_t logoHeight = _imageLogo.IsValid() ? _imageLogo.Height() : 0;
  
  // Move logo indexes
  int16_t logo_x = _lastLogo_x + _xDir;
  int16_t logo_y = _lastLogo_y + _yDir;

  // Move logo if image is available
  _imageLogo.Move(_lastLogo_x, _lastLogo_y, logo_x, logo_y, _tft, Config.tftColorBackground);
  
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
  for (uint8_t index = 0; index < SCREENSAVER_STARCOUNT; index++)
  {
    // Init new star, if star animation finished
    if (_stars[index].Size >= _stars[index].MaxSize)
    {
      // Clear old star only outside of the non-transparent part of the logo
      if (!_imageLogo.IsValid() ||
        !(_stars[index].X > logo_x && _stars[index].X < logo_x + logoWidth &&
        _stars[index].Y > logo_y && _stars[index].Y < logo_y + logoHeight &&
        _imageLogo.GetPixel(_stars[index].X - logo_x, _stars[index].Y - logo_y) != _imageLogo.TransparencyColor()))
      {
        DrawStar(_stars[index].X, _stars[index].Y, _stars[index].FullStars, Config.tftColorBackground, _stars[index].Size);
      }

      _stars[index].X = random(0, TFT_WIDTH);
      _stars[index].Y = random(0, TFT_HEIGHT);
      _stars[index].MaxSize = random(1, 6);
      _stars[index].FullStars = random(0, 12) < 6 ? true : false;
      _stars[index].Size = 0;
    }

    // Draw new star only outside of the non-transparent part of the logo
    if (!_imageLogo.IsValid() || 
      !(_stars[index].X > logo_x && _stars[index].X < logo_x + logoWidth &&
      _stars[index].Y > logo_y && _stars[index].Y < logo_y + logoHeight &&
      _imageLogo.GetPixel(_stars[index].X - logo_x, _stars[index].Y - logo_y) != _imageLogo.TransparencyColor()))
    {
      DrawStar(_stars[index].X, _stars[index].Y, _stars[index].FullStars, Config.tftColorForeground, _stars[index].Size);
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
// Draws a part of the bar
//===============================================================
void DisplayDriver::DrawBarPart(int16_t x0, int16_t y, MixtureLiquid liquid, BarBottle barBottle, BarBottle lastDraw_barBottle, int16_t liquidPercentage, int16_t lastDraw_liquidPercentage, String name, uint16_t color, bool isDashboard, bool isfullUpdate)
{
  int16_t namesOffsetX = 15;
  int16_t namesOffsetY = 175;

  MixtureLiquid dashboardLiquid = Statemachine.GetDashboardLiquid();
  BarBottle barBottle1 = Statemachine.GetBarBottle(0);
  BarBottle barBottle2 = Statemachine.GetBarBottle(1);
  BarBottle barBottle3 = Statemachine.GetBarBottle(2);
  
  bool isEmpty = barBottle == eEmpty;
  bool selectedChanged = dashboardLiquid != _lastDraw_SelectedLiquid;
  bool bottleChanged = barBottle != lastDraw_barBottle;
  bool sparklingWaterChanged = liquidPercentage != lastDraw_liquidPercentage;
  bool isSelected = dashboardLiquid == liquid;
  bool wasSelected = _lastDraw_SelectedLiquid == liquid;
  bool hasSparklingWater = barBottle1 == eSparklingWater || barBottle2 == eSparklingWater || barBottle3 == eSparklingWater;

  // Reset old bottle type selection -> only if bottle type changed
  if (bottleChanged)
  {
    SelectBarBottle(lastDraw_barBottle, x0, y, Config.tftColorBackground);
  }

  // Reset current bottle type selection -> only if selection changed and was selected
  if (selectedChanged && wasSelected)
  {
    SelectBarBottle(barBottle, x0, y, Config.tftColorBackground);
  }

  // Clear current bottle type -> only if bottle type changed
  if (bottleChanged)
  {
    ClearBarBottle(lastDraw_barBottle, barBottle, x0, y, Config.tftColorBackground);
  }
  
  // Draw current bottle selection -> only if selected AND (full update OR bottle changed OR selected changed)
  if (isSelected &&
    (isfullUpdate || bottleChanged || selectedChanged))
  {
    SelectBarBottle(barBottle, x0, y, Config.tftColorForeground);
  }
  
  // Draw current bottle and checkbox -> only if full update or bottle type changed
  if (isfullUpdate || bottleChanged)
  {
    // Draw bottle image
    DrawBarBottle(barBottle, x0, y);
  }

  // Draw liquid name -> only if full update, bottle type changed or selected changed
  if (isfullUpdate || bottleChanged || selectedChanged)
  {
    // Draw liquid name
    _tft->setTextColor(color);
    _tft->fillRect(x0 - namesOffsetX - 17, y + namesOffsetY - 15, 54, 30, Config.tftColorBackground);
    DrawCenteredString(name, x0 - namesOffsetX, y + namesOffsetY);
  }

  // Draw sparkling water percentage -> only if any sparkling water is present and not beside itself
  if (isDashboard &&
    !isEmpty &&
    hasSparklingWater &&
    barBottle != eSparklingWater &&
    (isfullUpdate || sparklingWaterChanged))
  {
    int16_t x = x0 - 37;
    int16_t yTop = y + namesOffsetY - 120;

    // Draw bar graph
    _tft->fillRect(x, yTop, 3, 100 - liquidPercentage, color);
    _tft->fillRect(x, yTop + 100 - liquidPercentage, 3, liquidPercentage, Config.tftColorForeground);

    // Draw percentage
    _tft->fillRect(x - 10, yTop - 15, 27, 20, Config.tftColorBackground);
    _tft->setTextColor(color);
    DrawCenteredString(String(liquidPercentage), x + 3, yTop - 5);
  }
}

//===============================================================
// Clears the difference from a bar bottle to the next bottle
//===============================================================
void DisplayDriver::ClearBarBottle(BarBottle lastDraw_barBottle, BarBottle barBottle, int16_t x0, int16_t y, uint16_t clearColor)
{
  if (lastDraw_barBottle == eEmpty)
  {
    return;
  }

  // Determine image pointers
  SPIFFSBMPImage* barBottlePointerLast = GetBarBottlePointer(lastDraw_barBottle);
  SPIFFSBMPImage* barBottlePointerNew = GetBarBottlePointer(barBottle);

  if (barBottlePointerLast &&
    barBottlePointerNew &&
    barBottlePointerLast->IsValid() &&
    barBottlePointerNew->IsValid())
  {
    // Clear difference from last to new image
    int16_t xLast = x0 - barBottlePointerLast->Width() / 2;
    int16_t xNew = x0 - barBottlePointerNew->Width() / 2;
    barBottlePointerLast->ClearDiff(xLast, y, xNew, y, barBottlePointerNew, _tft, clearColor);
  }
}

//===============================================================
// Draws a bar bottle
//===============================================================
void DisplayDriver::DrawBarBottle(BarBottle barBottle, int16_t x0, int16_t y)
{  
  // Determine correct pointer
  SPIFFSBMPImage* barBottlePointer = GetBarBottlePointer(barBottle);

  if (barBottlePointer &&
    barBottlePointer->IsValid())
  {
    // Draw bottle
    int16_t x = x0 - barBottlePointer->Width() / 2;
    barBottlePointer->Draw(x, y, _tft, Config.tftColorBackground, barBottle == eEmpty); // Use red wine bottle for empty selection (draw as shadow -> black)
  }
}

//===============================================================
// Draws a selection around a bar bottle
//===============================================================
void DisplayDriver::SelectBarBottle(BarBottle barBottle, int16_t x0, int16_t y, uint16_t color)
{
  // Determine correct pointer
  SPIFFSBMPImage* barBottlePointer = GetBarBottlePointer(barBottle);

  if (barBottlePointer &&
    barBottlePointer->IsValid())
  {
    // Draw selection shadow with move function
    int16_t selectionWidth = 3;
    int16_t x = x0 - barBottlePointer->Width() / 2;
    barBottlePointer->Move(x - selectionWidth, y, x, y, _tft, color, true);
    barBottlePointer->Move(x + selectionWidth, y, x, y, _tft, color, true);
    barBottlePointer->Move(x, y - selectionWidth, x, y, _tft, color, true);
  }
}

//===============================================================
// Returns a pointer to the requested bar bottle image
//===============================================================
SPIFFSBMPImage* DisplayDriver::GetBarBottlePointer(BarBottle barBottle)
{
  switch (barBottle)
  {
    case eWhiteWine:
      return &_imageBottle2;
    case eRoseWine:
      return &_imageBottle3;
    case eSparklingWater:
      return &_imageBottle4;
    case eRedWine:
    case eEmpty:
    default:
      return &_imageBottle1;
  }
}

//===============================================================
// Returns the settings name as string
//===============================================================
String DisplayDriver::GetSettingsName(MixerSetting setting)
{
  switch (setting)
  {
    case ePWM:
      return "PWM Cycle Time:";
    case eWLAN:
      return "WIFI Mode:";
    case eConfig:
      return "Config:";
    case eLEDIdle:
      return "LED Idle:";
    case eLEDDispensing:
      return "LED Dispense:";
    case eEncoder:
      return "Encoder:";
    case eScreen:
      return "Screen Saver:";
    default:
      return "Unknown";
  }
}

//===============================================================
// Returns the settings value as string
//===============================================================
String DisplayDriver::GetSettingsValue(MixerSetting setting)
{
  switch (setting)
  {
    case ePWM:
      return String(Pumps.GetCycleTimespan()) + "ms";
    case eWLAN:
      return Wifihandler.GetWifiMode() == WIFI_MODE_AP ? "AP" : "OFF";
    case eConfig:
      return Config.GetCurrent();
    case eLEDIdle:
    case eLEDDispensing:
      switch (setting == eLEDIdle ? Config.ledModeIdle : Config.ledModeDispensing)
      {
        case eOff:
          return "Off";
        case eOn:
          return "On";
        case eSlow:
          return "Blink Slow";
        case eFast:
          return "Blink Fast";
        case eFadingSlow:
          return "Fade Slow";
        case eFadingFast:
          return "Fade Fast";
      }
      break;
    case eEncoder:
      return Config.encoderDirection == 1 ? "Normal" : "Inverted";
    case eScreen:
      switch (Config.screenSaverMode)
      {
        case eNone:
          return "Off";
        case e2s:
          return "2s";
        case e15s:
          return "15s";
        case e30s:
          return "30s";
        case e1min:
          return "1min";
        case e5min:
          return "5min";
      }
      break;
  }

  return "???";
}

//===============================================================
// Draws a single setting
//===============================================================
void DisplayDriver::DrawSettingsEntry(int16_t x, int16_t y, const String &name, const String &value, bool selected, bool clear, bool isfullUpdate)
{
  if (isfullUpdate)
  {
    // Draw settings name
    _tft->setTextColor(clear ? Config.tftColorBackground : Config.tftColorTextBody);
    _tft->setCursor(x, y);
    _tft->print(name);
  }

  // Get text bounds
  int16_t x1, y1;
  uint16_t w, h;
  _tft->getTextBounds(name, x, y, &x1, &y1, &w, &h);

  // Draw settings value
  _tft->setCursor(x + w + 5, y);
  _tft->setTextColor(clear ? Config.tftColorBackground : selected ? Config.tftColorTextHeader : Config.tftColorTextBody);
  _tft->print(value);
}

//===============================================================
// Draws a string centered
//===============================================================
void DisplayDriver::DrawCenteredString(const String& text, int16_t x, int16_t y, bool underlined, uint16_t lineColor, bool backGround, uint16_t backGroundColor)
{
  // Get text bounds
  int16_t x1, y1;
  uint16_t w, h;
  _tft->getTextBounds(text, x, y, &x1, &y1, &w, &h);

  // Calculate cursor position
  int16_t x_text = x - w / 2;
  int16_t y_text = y + h / 2;
  _tft->setCursor(x_text, y_text);

  // Draw background if active
  if (backGround)
  {
    _tft->fillRect(x_text - 2, y - h /2, w + 4, h + 4, backGroundColor);
  }

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
String DisplayDriver::FormatValue(double value, int16_t mainPlaces, uint16_t decimalPlaces)
{
  String returnValue = "";

  dtostrf(value, mainPlaces, decimalPlaces, _output);
  returnValue += String(_output);

  return returnValue;
}
