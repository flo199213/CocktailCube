/**
 * Includes all system helper functions
 *
 * @author    Florian Staeblein
 * @date      2024/01/28
 * @copyright © 2024 Florian Staeblein
 */

//===============================================================
// Inlcudes
//===============================================================
#include "SystemHelper.h"

//===============================================================
// Constants
//===============================================================
static const char* TAG = "systemhelper";

//===============================================================
// Global variables
//===============================================================
SystemHelper Systemhelper;

//===============================================================
// Initializes the system helper
//===============================================================
void SystemHelper::Begin()
{
  // Log startup info
  ESP_LOGI(TAG, "Begin initializing system helper");

  // Show system information
  ESP_LOGI(TAG, "%s", GetSystemInfoString().c_str());

  // Print restart reason
  ESP_LOGI(TAG, "CPU0 reset reason: %s", GetResetReasonString(0).c_str());

  // Reset timestamp of last user action
  _lastUserAction = millis();
  
  // Log startup info
  ESP_LOGI(TAG, "Finished initializing system helper");
}

//===============================================================
// Returns the complete system info as string
//===============================================================
String SystemHelper::GetSystemInfoString()
{
  String returnString;
  
  uint32_t chipId = 0;
	for (int i = 0; i < 17; i = i + 8)
  {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
	}
  
  // Chip-Information
  returnString += "** Chip-Information: **\n";
  returnString += "Chip-ID:         0x" + String(chipId, HEX) + "\n";
  returnString += "Model:           " + String(ESP.getChipModel()) + "\n";
  returnString += "Revision:        " + String(ESP.getChipRevision()) + "\n";
  returnString += "SDK Version:     " + String(ESP.getSdkVersion()) + "\n";
  returnString += "\n";

  // CPU-Information
  returnString += "** CPU-Information: **\n";
  returnString += "CPU-Frequency:   " + String(ESP.getCpuFreqMHz()) + " MHz\n";
  returnString += "CPU Count:       " + String(ESP.getChipCores()) + "\n";
  returnString += "\n";

  // WLAN-Information
  returnString += "** WLAN-Information: **\n";
  returnString += "MAC:             " + String(WiFi.macAddress()) + "\n";
  returnString += "SSID:            " + String(WiFi.SSID()) + "\n";
  returnString += "BSSID:           " + String(WiFi.BSSIDstr()) + "\n";
  returnString += "Channel:         " + String(WiFi.channel()) + "\n";
  returnString += "TX Power:        " + WifiPowerToString(WiFi.getTxPower()) + "\n";
  returnString += "\n";

  // Memory-Tnformation
  returnString += "** Memory-Information: **\n";
  returnString += GetMemoryInfoString(true);
  returnString += "\n";

  return returnString;
}

//===============================================================
// Returns memory info string
//===============================================================
String SystemHelper::GetMemoryInfoString(bool all)
{
  String returnString;

  double spiffsTotal = max(1.0, (double)SPIFFS.totalBytes());
  double spiffsUsed = (double)SPIFFS.usedBytes();
  double spiffsUsage = spiffsUsed / spiffsTotal * 100.0;

  if (all)
  {
    returnString += "Flash-Size:      " + String((double)ESP.getFlashChipSize() / (1024.0 * 1024.0), 6) + " MB\n";
    returnString += "SRAM-Size:       " + String((double)ESP.getFreeHeap() / (1024.0 * 1024.0), 6) + " MB\n";
    returnString += "PRAM-Size:       " + String((double)ESP.getPsramSize() / (1024.0 * 1024.0), 6) + " MB\n";
    returnString += "\n";
    returnString += "Sketch-Size:     " + String((double)ESP.getSketchSize() / (1024.0 * 1024.0), 6) + " MB\n";
    returnString += "FreeSketch-Size: " + String((double)ESP.getFreeSketchSpace() / (1024.0 * 1024.0), 6) + " MB\n";
    returnString += "\n";
    returnString += "SPIFFS Ready:    " + String(SPIFFS.begin(false) ? "true\n" : "false\n");
    returnString += "SPIFFS-Total:    " + String(spiffsTotal / (1024.0 * 1024.0), 6) + " MB\n";
    returnString += "SPIFFS-Used:     " + String(spiffsUsed / (1024.0 * 1024.0), 6) + " MB (" + spiffsUsage + "%)\n";
    returnString += "Free-Heap:       " + String((double)ESP.getFreeHeap() / (1024.0 * 1024.0), 6) + " MB\n";
  }
  else
  {
    returnString += "SPIFFS-Used: " + String(spiffsUsed / (1024.0 * 1024.0), 6) + " MB (" + spiffsUsage + "%), ";
    returnString += "Free-Heap: " + String((double)ESP.getFreeHeap() / (1024.0 * 1024.0), 6) + " MB\n";
  }

  return returnString;
}

//===============================================================
// Returns a string for a wifi power
//===============================================================
String SystemHelper::WifiPowerToString(wifi_power_t power)
{
  switch (power)
  {
    case WIFI_POWER_19_5dBm:
      return "19.5 dBm";
    case WIFI_POWER_19dBm:
      return "19 dBm";
    case WIFI_POWER_18_5dBm:
      return "18.5 dBm";
    case WIFI_POWER_17dBm:
      return "17 dBm";
    case WIFI_POWER_15dBm:
      return "15 dBm";
    case WIFI_POWER_13dBm:
      return "13 dBm";
    case WIFI_POWER_11dBm:
      return "11 dBm";
    case WIFI_POWER_8_5dBm:
      return "8.5 dBm";
    case WIFI_POWER_7dBm:
      return "7 dBm";
    case WIFI_POWER_5dBm:
      return "5 dBm";
    case WIFI_POWER_2dBm:
      return "2 dBm";
    case WIFI_POWER_MINUS_1dBm:
      return "-1 dBm";
    default:
      return "Unknown";
  }
}

//===============================================================
// Returns the reset reason as string
//===============================================================
String SystemHelper::GetResetReasonString(int cpu)
{
  int reason = rtc_get_reset_reason(cpu);
  
  switch ( reason)
  {
    case 1:
      return "POWERON_RESET (Vbat power on reset)";
    case 3:
      return "SW_RESET (Software reset digital core)";
    case 4:
      return "OWDT_RESET (Legacy watch dog reset digital core)";
    case 5:
      return "DEEPSLEEP_RESET (Deep Sleep reset digital core)";
    case 6:
      return "SDIO_RESET (Reset by SLC module, reset digital core)";
    case 7:
      return "TG0WDT_SYS_RESET (Timer Group0 Watch dog reset digital core)";
    case 8:
      return "TG1WDT_SYS_RESET (Timer Group1 Watch dog reset digital core)";
    case 9:
      return "RTCWDT_SYS_RESET (RTC Watch dog Reset digital core)";
    case 10:
      return "INTRUSION_RESET (Instrusion tested to reset CPU)";
    case 11:
      return "TGWDT_CPU_RESET (Time Group reset CPU)";
    case 12:
      return "SW_CPU_RESET (Software reset CPU)";
    case 13:
      return "RTCWDT_CPU_RESET (RTC Watch dog Reset CPU)";
    case 14:
      return "EXT_CPU_RESET (for APP CPU, reseted by PRO CPU)";
    case 15:
      return "RTCWDT_BROWN_OUT_RESET (Reset when the vdd voltage is not stable)";
    case 16:
      return "RTCWDT_RTC_RESET (RTC Watch dog reset digital core and rtc module)";
    default:
      return "NO_MEAN";
  }
}

//===============================================================
// Sets the timestamp of the last user action to the current time
//===============================================================
void SystemHelper::SetLastUserAction()
{
  _lastUserAction = millis();
}

//===============================================================
// Return the timestamp of the last user action
//===============================================================
uint32_t SystemHelper::GetLastUserAction()
{
  return _lastUserAction;
}
