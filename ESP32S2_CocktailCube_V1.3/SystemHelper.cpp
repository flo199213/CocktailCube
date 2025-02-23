/*
 * Includes all system helper functions
 *
 * @author    Florian Stäblein
 * @date      2025/01/01
 * @copyright © 2025 Florian Stäblein
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
	for (uint8_t i = 0; i < 17; i = i + 8)
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

  double heapTotal = max(1.0, (double)ESP.getHeapSize());
  double heapUsed = heapTotal - (double)ESP.getFreeHeap();
  double heapUsage = heapUsed / heapTotal * 100.0;

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
    returnString += "Heap-Total:      " + String(heapTotal / (1024.0 * 1024.0), 6) + " MB\n";
    returnString += "Heap-Used:       " + String(heapUsed / (1024.0 * 1024.0), 6) + " MB (" + heapUsage + "%)\n";
  }
  else
  {
    returnString += "SPIFFS-Used: " + String(spiffsUsed / (1024.0 * 1024.0), 6) + " MB (" + spiffsUsage + "%), ";
    returnString += "Heap-Used: " + String(heapUsed / (1024.0 * 1024.0), 6) + " MB (" + heapUsage + "%)\n";
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
String SystemHelper::GetResetReasonString(int8_t cpu)
{
  RESET_REASON reason = rtc_get_reset_reason(cpu);

  switch (reason)
  {
    case POWERON_RESET:
      return "POWERON_RESET (Vbat power on reset)";
    case RTC_SW_SYS_RESET:
      return "SW_RESET (Software reset digital core)";
    case DEEPSLEEP_RESET:
      return "DEEPSLEEP_RESET (Deep Sleep reset digital core)";
    case TG0WDT_SYS_RESET:
      return "TG0WDT_SYS_RESET (Timer Group0 Watch dog reset digital core)";
    case TG1WDT_SYS_RESET:
      return "TG1WDT_SYS_RESET (Timer Group1 Watch dog reset digital core)";
    case RTCWDT_SYS_RESET:
      return "RTCWDT_SYS_RESET (RTC Watch dog Reset digital core)";
    case INTRUSION_RESET:
      return "INTRUSION_RESET (Instrusion tested to reset CPU)";
    case TG0WDT_CPU_RESET:
      return "TGWDT_CPU_RESET (Time Group reset CPU)";
    case RTC_SW_CPU_RESET:
      return "SW_CPU_RESET (Software reset CPU)";
    case RTCWDT_CPU_RESET:
      return "RTCWDT_CPU_RESET (RTC Watch dog Reset CPU)";
    case RTCWDT_BROWN_OUT_RESET:
      return "RTCWDT_BROWN_OUT_RESET (Reset when the vdd voltage is not stable)";
    case RTCWDT_RTC_RESET:
      return "RTCWDT_RTC_RESET (RTC Watch dog reset digital core and rtc module)";
    case TG1WDT_CPU_RESET:
      return "TG1WDT_CPU_RESET (Time Group1 reset CPU)";
    case SUPER_WDT_RESET:
      return "SUPER_WDT_RESET (super watchdog reset digital core and rtc module)";
    case GLITCH_RTC_RESET:
      return "GLITCH_RTC_RESET (glitch reset digital core and rtc module)";
    case EFUSE_RESET:
      return "EFUSE_RESET (efuse reset digital core)";
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
