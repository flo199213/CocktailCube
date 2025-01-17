/**
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

  // Load startup image to RAM: Logo
  _imageLogo = new SPIFFSFLOImage();
  ImageReturnCode returnCode = _imageLogo->Allocate(config.imageLogo);
  ESP_LOGI(TAG, "Load image 'Logo': %s (Heap: %d / %d Bytes)", _imageLogo->PrintStatus(returnCode).c_str(), ESP.getFreeHeap(), ESP.getHeapSize());

  // Load startup image to RAM: Glass
  _imageGlass = new SPIFFSFLOImage();
  returnCode = _imageGlass->Allocate(config.imageGlass);
  ESP_LOGI(TAG, "Load image 'Glass': %s (Heap: %d / %d Bytes)", _imageGlass->PrintStatus(returnCode).c_str(), ESP.getFreeHeap(), ESP.getHeapSize());

  // Load startup images to RAM: Bottle 1
  _imageBottle1 = new SPIFFSFLOImage();
  returnCode = _imageBottle1->Allocate(config.imageBottle1);
  ESP_LOGI(TAG, "Load image 'Bottle 1': %s (Heap: %d / %d Bytes)", _imageBottle1->PrintStatus(returnCode).c_str(), ESP.getFreeHeap(), ESP.getHeapSize());

  if (!config.isMixer)
  {
    // Load startup images to RAM: Bottle 2
    _imageBottle2 = new SPIFFSFLOImage();
    returnCode = _imageBottle2->Allocate(config.imageBottle2);
    ESP_LOGI(TAG, "Load image 'Bottle 2': %s (Heap: %d / %d Bytes)", _imageBottle2->PrintStatus(returnCode).c_str(), ESP.getFreeHeap(), ESP.getHeapSize());
    
    // Load startup images to RAM: Bottle 3
    _imageBottle3 = new SPIFFSFLOImage();
    returnCode = _imageBottle3->Allocate(config.imageBottle3);
    ESP_LOGI(TAG, "Load image 'Bottle 3': %s (Heap: %d / %d Bytes)", _imageBottle3->PrintStatus(returnCode).c_str(), ESP.getFreeHeap(), ESP.getHeapSize());
    
    // Load startup images to RAM: Bottle 4
    _imageBottle4 = new SPIFFSFLOImage();
    returnCode = _imageBottle4->Allocate(config.imageBottle4);
    ESP_LOGI(TAG, "Load image 'Bottle 4': %s (Heap: %d / %d Bytes)", _imageBottle4->PrintStatus(returnCode).c_str(), ESP.getFreeHeap(), ESP.getHeapSize());
  }

  // Log info
  ESP_LOGI(TAG, "Finished loading images");
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
// Sets the bar stock
//===============================================================
void DisplayDriver::SetBar(BarBottle barBottle1, BarBottle barBottle2, BarBottle barBottle3)
{
  _barBottle1 = barBottle1;
  _barBottle2 = barBottle2;
  _barBottle3 = barBottle3;
}

//===============================================================
// Shows intro page
//===============================================================
void DisplayDriver::ShowIntroPage()
{
  // Set log
  ESP_LOGI(TAG, "Show intro page");

  // Draw intro page background
  _tft->fillRect(0, 0,                TFT_WIDTH, TFT_HEIGHT * 0.8, config.tftColorStartPageBackground);
  _tft->fillRect(0, TFT_HEIGHT * 0.8, TFT_WIDTH, TFT_HEIGHT * 0.2, config.tftColorStartPageForeground);

  // Draw intro images (Attention: Order is decisive!)
  if (_imageBottle1 &&
    _imageGlass &&
    _imageLogo &&
    _imageBottle1->IsValid() &&
    _imageGlass->IsValid() &&
    _imageLogo->IsValid())
  {
    _imageBottle1->Draw(config.tftBottlePosX, config.tftBottlePosY, _tft);
    _imageGlass->Draw(config.tftGlassPosX,    config.tftGlassPosY,  _tft);
    _imageLogo->Draw(config.tftLogoPosX,      config.tftLogoPosY,   _tft);
  }
  else
  {
    // Draw info box (fallback)
    DrawInfoBox("- Startpage -", "No SPIFFS Files!");
  }

  // Do NOT delete logo image for usage for screensaver !
  /*if (_imageLogo)
  {
    _imageLogo->Deallocate();
    _imageLogo = NULL;
    ESP_LOGI(TAG, "Image 'Logo' deleted (Heap: %d / %d Bytes)", ESP.getFreeHeap(), ESP.getHeapSize());
  }*/
  
  if (_imageGlass)
  {
    _imageGlass->Deallocate();
    _imageGlass = NULL;
    ESP_LOGI(TAG, "Image 'Glass' deleted (Heap: %d / %d Bytes)", ESP.getFreeHeap(), ESP.getHeapSize());
  }

  // Do NOT delete bottle images for usage for bar display !
  if (config.isMixer)
  {
    if (_imageBottle1)
    {
      _imageBottle1->Deallocate();
      _imageBottle1 = NULL;
      ESP_LOGI(TAG, "Image 'Bottle 1' deleted (Heap: %d / %d Bytes)", ESP.getFreeHeap(), ESP.getHeapSize());
    }
    if (_imageBottle2)
    {
      _imageBottle2->Deallocate();
      _imageBottle2 = NULL;
      ESP_LOGI(TAG, "Image 'Bottle 2' deleted (Heap: %d / %d Bytes)", ESP.getFreeHeap(), ESP.getHeapSize());
    }
    if (_imageBottle3)
    {
      _imageBottle3->Deallocate();
      _imageBottle3 = NULL;
      ESP_LOGI(TAG, "Image 'Bottle 3' deleted (Heap: %d / %d Bytes)", ESP.getFreeHeap(), ESP.getHeapSize());
    }
    if (_imageBottle4)
    {
      _imageBottle4->Deallocate();
      _imageBottle4 = NULL;
      ESP_LOGI(TAG, "Image 'Bottle 4' deleted (Heap: %d / %d Bytes)", ESP.getFreeHeap(), ESP.getHeapSize());
    }
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
  _tft->fillScreen(config.tftColorBackground);
  
  // Draw header information
  DrawHeader("Instructions", false);

  // Set text settings
  _tft->setTextSize(1);
  _tft->setTextColor(config.tftColorTextBody);

  // Draw help text
  _tft->setCursor(x, y);
  _tft->print("Short Press:");
  _tft->setCursor(x, y += SHORTLINEOFFSET);
  _tft->print(" -> Change Setting");
  _tft->setCursor(x, y += SHORTLINEOFFSET);
  _tft->print("    ~ ");
  _tft->print(config.liquid1Name);
  _tft->setCursor(x, y += SHORTLINEOFFSET);
  _tft->print("    ~ ");
  _tft->print(config.liquid2Name);
  _tft->setCursor(x, y += SHORTLINEOFFSET);
  _tft->print("    ~ ");
  _tft->print(config.liquid3Name);
  
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
  _tft->fillScreen(config.tftColorBackground);
  
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
  _tft->fillScreen(config.tftColorBackground);
  
  // Draw header information
  DrawHeader();
  
  if (config.isMixer)
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
    _tft->setTextColor(config.tftColorForeground);
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
  _tft->fillScreen(config.tftColorBackground);
  
  // Draw header information
  DrawHeader("Cleaning Mode");

  int16_t x = TFT_WIDTH / 2;
  int16_t y = TFT_HEIGHT / 3;
  
  // Print selection text
  _tft->setTextColor(config.tftColorForeground);
  DrawCenteredString("Select pumps for cleaning:", x, y);

  // Draw checkboxes
  DrawCheckBoxes(_cleaningLiquid);
}

//===============================================================
// Shows bar page
//===============================================================
void DisplayDriver::ShowBarPage()
{
  // Clear screen
  _tft->fillScreen(config.tftColorBackground);
  
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
  _tft->fillScreen(config.tftColorBackground);
  
  // Draw header information
  DrawHeader("Settings");

  double valueLiquid1 = FlowMeter.GetValueLiquid1();
  double valueLiquid2 = FlowMeter.GetValueLiquid2();
  double valueLiquid3 = FlowMeter.GetValueLiquid3();

  // Fill in settings text
  _tft->setTextSize(1);
  _tft->setTextColor(config.tftColorTextBody);

  _tft->setCursor(x, y);
  _tft->print("App Version: ");
  _tft->print(APP_VERSION);

  DrawSettings(true);

  _tft->setCursor(x, y += (SHORTLINEOFFSET + 2 * LONGLINEOFFSET));
  _tft->print("Volume of liquid filled:");
  
  // Draw liquid 1 flow meter value
  _tft->setTextColor(config.tftColorLiquid1);
  _tft->setCursor(x, y += SHORTLINEOFFSET);
  _tft->print(config.liquid1Name);
  _tft->print(":");
  _tft->setCursor(x + 120, y);
  _tft->print(FormatValue(valueLiquid1, 4, 2));
  _tft->print(" L");
  
  // Draw liquid 2 flow meter value
  _tft->setTextColor(config.tftColorLiquid2);
  _tft->setCursor(x, y += SHORTLINEOFFSET);
  _tft->print(config.liquid2Name);
  _tft->print(":");
  _tft->setCursor(x + 120, y);
  _tft->print(FormatValue(valueLiquid2, 4, 2));
  _tft->print(" L");  
  
  // Draw liquid 3 flow meter value
  _tft->setTextColor(config.tftColorLiquid3);
  _tft->setCursor(x, y += SHORTLINEOFFSET);
  _tft->print(config.liquid3Name);
  _tft->print(":");
  _tft->setCursor(x + 120, y);
  _tft->print(FormatValue(valueLiquid3, 4, 2));
  _tft->print(" L");
  
  x = 40;
  y = TFT_HEIGHT - 20;

  // Draw copyright icon
  _tft->drawXBitmap(x, y, icon_copyright, 20, 20, config.tftColorTextBody);
  
  // Draw copyright text
  _tft->setCursor(x + 25, y + 15);
  _tft->setTextColor(config.tftColorTextBody);
  _tft->print(APP_COPYRIGHT);
  _tft->print(" F.Stablein");
  _tft->drawRect(x + 105, y + 2, 2, 2, config.tftColorTextBody);  // Stablein with two dots -> Stäblein
  _tft->drawRect(x + 109, y + 2, 2, 2, config.tftColorTextBody);  // Stablein with two dots -> Stäblein
}

//===============================================================
// Shows screen saver page
//===============================================================
void DisplayDriver::ShowScreenSaverPage()
{
  // Set log
  ESP_LOGI(TAG, "Show screen saver page");

  // Clear screen
  _tft->fillScreen(config.tftColorBackground);

  // Draw inital screen saver
  DrawScreenSaver();
}

//===============================================================
// Draws default header Text
//===============================================================
void DisplayDriver::DrawHeader()
{
  DrawHeader(String("-- ") + config.mixerName + " --");
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
  _tft->setTextColor(config.tftColorTextHeader);
  DrawCenteredString(text, x, y);

  x = HEADER_MARGIN;
  y = HEADEROFFSET_Y;
  int16_t x1 = TFT_WIDTH - (2 * HEADER_MARGIN);
  int16_t y1 = HEADEROFFSET_Y;

  // Draw header line
  _tft->drawLine(x, y, x1, y1, config.tftColorForeground);

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
  _tft->drawXBitmap(x, y, icon_wifi, width, height, config.tftColorBackground);
  _tft->drawXBitmap(x, y, icon_noWifi, width, height, config.tftColorBackground);

  // Draw new wifi icon
  _tft->drawXBitmap(x, y, wifiMode == WIFI_MODE_AP ? icon_wifi : icon_noWifi, width, height, config.tftColorForeground);

  x = 5;
  y += 2;

  // Clear connected clients
  _tft->fillRect(x, y, width, height, config.tftColorBackground);

  if (wifiMode == WIFI_MODE_AP)
  {
    // Draw new connected clients
    _tft->drawXBitmap(x, y, icon_device, width, height, config.tftColorForeground);
    _tft->setCursor(x + 7, y + 17);
    _tft->setTextColor(config.tftColorForeground);
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
  _tft->fillRoundRect(x,     y,      width,     height,     INFOBOX_CORNERRADIUS, config.tftColorInfoBoxBorder);
  _tft->fillRoundRect(x + 2, y + 2,  width - 4, height - 4, INFOBOX_CORNERRADIUS, config.tftColorInfoBoxBackground);
  
  // Move to the middle of the box
  x += width / 2;
  y += height / 2;

  // Fill in info text
  _tft->setTextSize(1);
  _tft->setTextColor(config.tftColorInfoBoxForeground);
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
    _tft->drawXBitmap(x, y,                    icon_dashboard, width, height, config.tftColorForeground);
    _tft->drawXBitmap(x, y += MENU_LINEOFFSET, icon_cleaning,  width, height, config.tftColorForeground);
    _tft->drawXBitmap(x, y += MENU_LINEOFFSET, icon_reset,     width, height, config.tftColorForeground);
    _tft->drawXBitmap(x, y += MENU_LINEOFFSET, icon_settings,  width, height, config.tftColorForeground);

    x = MENU_MARGIN_HORI + MENU_MARGIN_ICON + MENU_MARGIN_TEXT;
    y = HEADEROFFSET_Y + marginToHeader;

    // Draw menu text
    _tft->setTextSize(1);
    _tft->setTextColor(config.tftColorTextBody);
    _tft->setCursor(x, y);
    _tft->print("Dashboard");
    _tft->setCursor(x, y += MENU_LINEOFFSET);
    _tft->print("Cleaning Mode");
    _tft->setCursor(x, y += MENU_LINEOFFSET);
    if (config.isMixer)
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

  if (_lastDraw_MenuState != _menuState || isfullUpdate)
  {
    uint16_t offsetIndex = (uint16_t)_lastDraw_MenuState - 1;
    if (!config.isMixer &&
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
    _tft->drawRoundRect(x, y, width, height, MENU_SELECTOR_CORNERRADIUS, config.tftColorBackground);

    offsetIndex = (uint16_t)_menuState - 1;
    if (!config.isMixer &&
      _menuState == eBar)
    {
      // Fix for bar stock (enum index 6)
      offsetIndex = 2;
    }

    y = HEADEROFFSET_Y + marginToHeader + offsetIndex * MENU_LINEOFFSET - 6 - MENU_SELECTOR_HEIGHT / 2;

    // Draw new menu selection on display
    _tft->drawRoundRect(x, y, width, height, MENU_SELECTOR_CORNERRADIUS, config.tftColorMenuSelector);

    // Save last state
    _lastDraw_MenuState = _menuState;
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

  // Draw only check boxes if complete bar stock is empty
  if (isDashboard &&
    _barBottle1 == eEmpty &&
    _barBottle2 == eEmpty &&
    _barBottle3 == eEmpty)
  {
    // Print selection text
    _tft->setTextColor(config.tftColorForeground);
    DrawCenteredString("Select WINE for dispensing:", x0, y + 25, false, 0, true, 0x528A); // Gray

    // Draw checkboxes
    DrawCheckBoxes(_dashboardLiquid);
  }
  else
  {
    // Draw each bottle
    DrawBarPart(x0 - spacing, y, eLiquid1, _barBottle1, _lastDraw_barBottle1, _liquid1_Percentage, _lastDraw_liquid1_Percentage, config.liquid1Name, config.tftColorLiquid1, isDashboard, isfullUpdate);
    DrawBarPart(x0,           y, eLiquid2, _barBottle2, _lastDraw_barBottle2, _liquid2_Percentage, _lastDraw_liquid2_Percentage, config.liquid2Name, config.tftColorLiquid2, isDashboard, isfullUpdate);
    DrawBarPart(x0 + spacing, y, eLiquid3, _barBottle3, _lastDraw_barBottle3, _liquid3_Percentage, _lastDraw_liquid3_Percentage, config.liquid3Name, config.tftColorLiquid3, isDashboard, isfullUpdate);

    if (isDashboard &&
      (isfullUpdate || _dashboardLiquid != _lastDraw_SelectedLiquid))
    {
      // Print selection text
      _tft->setTextColor(config.tftColorForeground);
      DrawCenteredString("Select WINE for dispensing:", x0, y + 25, false, 0, true, 0x528A); // Gray
    }

    _lastDraw_SelectedLiquid = _dashboardLiquid;
    _lastDraw_barBottle1 = _barBottle1;
    _lastDraw_barBottle2 = _barBottle2;
    _lastDraw_barBottle3 = _barBottle3;
    _lastDraw_liquid1_Percentage = _liquid1_Percentage;
    _lastDraw_liquid2_Percentage = _liquid2_Percentage;
    _lastDraw_liquid3_Percentage = _liquid3_Percentage;
  }

  ESP_LOGI(TAG, "_barBottles: %d %d %d", _barBottle1, _barBottle2, _barBottle3);
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
  _tft->drawRect(x0 - boxSize / 2 - spacing, y, boxSize, boxSize, config.tftColorForeground);
  _tft->drawRect(x0 - boxSize / 2,           y, boxSize, boxSize, config.tftColorForeground);
  _tft->drawRect(x0 - boxSize / 2 + spacing, y, boxSize, boxSize, config.tftColorForeground);
  
  // Draw activated checkboxes
  _tft->fillRect(x0 - boxSize / 2 + 4 - spacing, y + 4, boxSize - 8, boxSize - 8, liquid == eLiquidAll || liquid == eLiquid1 ? config.tftColorStartPage : config.tftColorBackground);
  _tft->fillRect(x0 - boxSize / 2 + 4,           y + 4, boxSize - 8, boxSize - 8, liquid == eLiquidAll || liquid == eLiquid2 ? config.tftColorStartPage : config.tftColorBackground);
  _tft->fillRect(x0 - boxSize / 2 + 4 + spacing, y + 4, boxSize - 8, boxSize - 8, liquid == eLiquidAll || liquid == eLiquid3 ? config.tftColorStartPage : config.tftColorBackground);

  // Move under the boxes for liquid names
  y = HEADEROFFSET_Y + 140;

  // Draw liquid names
  _tft->setTextColor(config.tftColorLiquid1);
  DrawCenteredString(config.liquid1Name, x0 - spacing, y);
  _tft->setTextColor(config.tftColorLiquid2);
  DrawCenteredString(config.liquid2Name, x0, y);
  _tft->setTextColor(config.tftColorLiquid3);
  DrawCenteredString(config.liquid3Name, x0 + spacing, y);
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
  _tft->drawRect(x, y, width, height, config.tftColorForeground);

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
  _tft->fillRect(x, y,                    width, height, config.tftColorLiquid1);
  _tft->fillRect(x, y += LOONGLINEOFFSET, width, height, config.tftColorLiquid2);
  _tft->fillRect(x, y += LOONGLINEOFFSET, width, height, config.tftColorLiquid3);

  // Move to inner text
  x = X_LEGEND + WIDTH_LEGEND / 2;
  y = Y_LEGEND + marginTop + marginBetween;

  // Draw liquid text
  _tft->setTextSize(1);
  _tft->setTextColor(config.tftColorTextBody);  
  DrawCenteredString(config.liquid1Name, x, y,                    true, _dashboardLiquid == eLiquid1 ? config.tftColorForeground : config.tftColorBackground, false, 0);
  DrawCenteredString(config.liquid2Name, x, y += LOONGLINEOFFSET, true, _dashboardLiquid == eLiquid2 ? config.tftColorForeground : config.tftColorBackground, false, 0);
  DrawCenteredString(config.liquid3Name, x, y += LOONGLINEOFFSET, true, _dashboardLiquid == eLiquid3 ? config.tftColorForeground : config.tftColorBackground, false, 0);
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
    _tft->setTextColor(config.tftColorTextBody);
    _tft->setCursor(x, y);
    _tft->print("Mix [");
  }

  x += 40;
  if (_lastDraw_Liquid1String != liquid1_PercentageString || isfullUpdate)
  {
    // Reset old string on display
    _tft->setTextColor(config.tftColorBackground);
    _tft->setCursor(x, y);
    _tft->print(_lastDraw_Liquid1String);
    
    // Draw new string on display
    _tft->setTextColor(config.tftColorLiquid1);
    _tft->setCursor(x, y);
    _tft->print(liquid1_PercentageString);
    
    // Save last drawn string
    _lastDraw_Liquid1String = liquid1_PercentageString;
  }

  x += 40;
  if (isfullUpdate)
  {
    _tft->setTextColor(config.tftColorTextBody);
    _tft->setCursor(x, y);
    _tft->print(",");
  }

  x += 10;
  if (_lastDraw_Liquid2String != liquid2_PercentageString || isfullUpdate)
  {
    // Reset old string on display
    _tft->setTextColor(config.tftColorBackground);
    _tft->setCursor(x, y);
    _tft->print(_lastDraw_Liquid2String);
    
    // Draw new string on display
    _tft->setTextColor(config.tftColorLiquid2);
    _tft->setCursor(x, y);
    _tft->print(liquid2_PercentageString);

    // Save last drawn string
    _lastDraw_Liquid2String = liquid2_PercentageString;
  }
  
  x += 40;
  if (isfullUpdate)
  {
    _tft->setTextColor(config.tftColorTextBody);
    _tft->setCursor(x, y);
    _tft->print(",");
  }
  
  x += 10;
  if (_lastDraw_Liquid3String != liquid3_PercentageString || isfullUpdate)
  {
    // Reset old string on display
    _tft->setTextColor(config.tftColorBackground);
    _tft->setCursor(x, y);
    _tft->print(_lastDraw_Liquid3String);
    
    // Draw new string on display
    _tft->setTextColor(config.tftColorLiquid3);
    _tft->setCursor(x, y);
    _tft->print(liquid3_PercentageString);

    // Save last drawn string
    _lastDraw_Liquid3String = liquid3_PercentageString;
  }

  x += 45;
  if (isfullUpdate)
  {
    _tft->setTextColor(config.tftColorTextBody);
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
    FillArc(_liquid1Angle_Degrees, liquid1Distance_Degrees, config.tftColorLiquid1);
    FillArc(_liquid2Angle_Degrees, liquid2Distance_Degrees, config.tftColorLiquid2);
    FillArc(_liquid3Angle_Degrees, liquid3Distance_Degrees, config.tftColorLiquid3);
  }
  else
  {
    DrawPartial(_liquid1Angle_Degrees, _lastDraw_liquid1Angle_Degrees, config.tftColorLiquid1, config.tftColorLiquid3, clockwise);
    DrawPartial(_liquid2Angle_Degrees, _lastDraw_liquid2Angle_Degrees, config.tftColorLiquid2, config.tftColorLiquid1, clockwise);
    DrawPartial(_liquid3Angle_Degrees, _lastDraw_liquid3Angle_Degrees, config.tftColorLiquid3, config.tftColorLiquid2, clockwise);
  }

  // Draw black spacer and selected white
  FillArc(Move360(_liquid1Angle_Degrees, -SPACERANGLE_DEGREES), 2 * SPACERANGLE_DEGREES, _dashboardLiquid == eLiquid1 ? config.tftColorForeground : config.tftColorBackground);
  FillArc(Move360(_liquid2Angle_Degrees, -SPACERANGLE_DEGREES), 2 * SPACERANGLE_DEGREES, _dashboardLiquid == eLiquid2 ? config.tftColorForeground : config.tftColorBackground);
  FillArc(Move360(_liquid3Angle_Degrees, -SPACERANGLE_DEGREES), 2 * SPACERANGLE_DEGREES, _dashboardLiquid == eLiquid3 ? config.tftColorForeground : config.tftColorBackground);
  
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
    _tft->setTextColor(config.tftColorTextBody);
    _tft->setCursor(x, y);
    _tft->print("PWM CycleTime: ");
  }

  uint32_t cycleTimespan_ms = Pumps.GetCycleTimespan();

  if (_lastDraw_cycleTimespan_ms != cycleTimespan_ms || isfullUpdate)
  {
    // Clear old value
    _tft->setCursor(x + 145, y);
    _tft->setTextColor(config.tftColorBackground);
    _tft->print(_lastDraw_cycleTimespan_ms);
    _tft->print(" ms");

    // Set new value
    _tft->setCursor(x + 145, y);
    _tft->setTextColor(config.tftColorTextBody);
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
    _tft->setTextColor(config.tftColorTextBody);
    _tft->setCursor(x, y);
    _tft->print("WIFI Mode: ");
  }

  if (_lastDraw_wifiMode != wifiMode || isfullUpdate)
  {
    // Clear old value
    _tft->setCursor(x + 98, y);
    _tft->setTextColor(config.tftColorBackground);
    _tft->print(_lastDraw_wifiMode == WIFI_MODE_AP ? "AP" : "OFF");

    // Set new value
    _tft->setCursor(x + 98, y);
    _tft->setTextColor(config.tftColorTextBody);
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
  bool logoAvailable = _imageLogo != NULL && _imageLogo->IsValid();

  int16_t logoWidth = logoAvailable ? _imageLogo->Width() : 0;
  int16_t logoHeight = logoAvailable ? _imageLogo->Height() : 0;
  
  // Move logo indexes
  int16_t logo_x = _lastLogo_x + _xDir;
  int16_t logo_y = _lastLogo_y + _yDir;

  // Move logo if image is available
  if (logoAvailable)
  {
    _imageLogo->Move(_lastLogo_x, _lastLogo_y, logo_x, logo_y, _tft, config.tftColorBackground);
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
      if (!logoAvailable ||
        !(_stars[index].X > logo_x && _stars[index].X < logo_x + logoWidth &&
        _stars[index].Y > logo_y && _stars[index].Y < logo_y + logoHeight &&
        _imageLogo->GetPixel(_stars[index].X - logo_x, _stars[index].Y - logo_y) != _imageLogo->TransparencyColor()))
      {
        DrawStar(_stars[index].X, _stars[index].Y, _stars[index].FullStars, config.tftColorBackground, _stars[index].Size);
      }

      _stars[index].X = random(0, TFT_WIDTH);
      _stars[index].Y = random(0, TFT_HEIGHT);
      _stars[index].MaxSize = random(1, 6);
      _stars[index].FullStars = random(0, 12) < 6 ? true : false;
      _stars[index].Size = 0;
    }

    // Draw new star only outside of the non-transparent part of the logo
    if (!logoAvailable || 
      !(_stars[index].X > logo_x && _stars[index].X < logo_x + logoWidth &&
      _stars[index].Y > logo_y && _stars[index].Y < logo_y + logoHeight &&
      _imageLogo->GetPixel(_stars[index].X - logo_x, _stars[index].Y - logo_y) != _imageLogo->TransparencyColor()))
    {
      DrawStar(_stars[index].X, _stars[index].Y, _stars[index].FullStars, config.tftColorForeground, _stars[index].Size);
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
void DisplayDriver::DrawBarPart(int16_t x0, int16_t y, MixtureLiquid liquid, BarBottle barBottle, BarBottle lastDraw_barBottle, int16_t liquid_Percentage, int16_t lastDraw_liquid_Percentage, String name, uint16_t color, bool isDashboard, bool isfullUpdate)
{
  int16_t namesOffsetX = 15;
  int16_t namesOffsetY = 175;
  
  bool isEmpty = barBottle == eEmpty;
  bool selectedChanged = _dashboardLiquid != _lastDraw_SelectedLiquid;
  bool bottleChanged = barBottle != lastDraw_barBottle;
  bool sparklingWaterChanged = liquid_Percentage != lastDraw_liquid_Percentage;
  bool isSelected = _dashboardLiquid == liquid;
  bool wasSelected = _lastDraw_SelectedLiquid == liquid;
  bool hasSparklingWater = _barBottle1 == eSparklingWater || _barBottle2 == eSparklingWater || _barBottle3 == eSparklingWater;

  // Reset old bottle type selection -> only if bottle type changed
  if (bottleChanged)
  {
    SelectBarBottle(lastDraw_barBottle, x0, y, config.tftColorBackground);
  }

  // Reset current bottle type selection -> only if selection changed and was selected
  if (selectedChanged && wasSelected)
  {
    SelectBarBottle(barBottle, x0, y, config.tftColorBackground);
  }

  // Clear current bottle type -> only if bottle type changed
  if (bottleChanged)
  {
    ClearBarBottle(lastDraw_barBottle, barBottle, x0, y, config.tftColorBackground);
  }
  
  // Draw current bottle selection -> only if selected AND (full update OR bottle changed OR selected changed)
  if (isSelected &&
    (isfullUpdate || bottleChanged || selectedChanged))
  {
    SelectBarBottle(barBottle, x0, y, config.tftColorForeground);
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
    _tft->fillRect(x0 - namesOffsetX - 17, y + namesOffsetY - 15, 54, 30, config.tftColorBackground);
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
    _tft->fillRect(x, yTop, 3, 100 - liquid_Percentage, color);
    _tft->fillRect(x, yTop + 100 - liquid_Percentage, 3, liquid_Percentage, config.tftColorForeground);

    // Draw percentage
    _tft->fillRect(x - 10, yTop - 15, 27, 20, config.tftColorBackground);
    _tft->setTextColor(color);
    DrawCenteredString(String(liquid_Percentage), x + 3, yTop - 5);
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
  SPIFFSFLOImage* barBottlePointerLast = GetBarBottlePointer(lastDraw_barBottle);
  SPIFFSFLOImage* barBottlePointerNew = GetBarBottlePointer(barBottle);

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
  SPIFFSFLOImage* barBottlePointer = GetBarBottlePointer(barBottle);

  if (barBottlePointer &&
    barBottlePointer->IsValid())
  {
    // Draw bottle
    int16_t x = x0 - barBottlePointer->Width() / 2;
    barBottlePointer->Draw(x, y, _tft, config.tftColorBackground, barBottle == eEmpty); // Use red wine bottle for empty selection (draw as shadow -> black)
  }
}

//===============================================================
// Draws a selection around a bar bottle
//===============================================================
void DisplayDriver::SelectBarBottle(BarBottle barBottle, int16_t x0, int16_t y, uint16_t color)
{
  // Determine correct pointer
  SPIFFSFLOImage* barBottlePointer = GetBarBottlePointer(barBottle);

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
SPIFFSFLOImage* DisplayDriver::GetBarBottlePointer(BarBottle barBottle)
{
  switch (barBottle)
  {
    case eWhiteWine:
      return _imageBottle2;
    case eRoseWine:
      return _imageBottle3;
    case eSparklingWater:
      return _imageBottle4;
    case eRedWine:
    case eEmpty:
    default:
      return _imageBottle1;
  }
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
String DisplayDriver::FormatValue(double value, int mainPlaces, int decimalPlaces)
{
  String returnValue = "";

  dtostrf(value, mainPlaces, decimalPlaces, _output);
  returnValue += String(_output);

  return returnValue;
}
