/**
 * Includes all system helper functions
 *
 * @author    Florian Staeblein
 * @date      2024/01/28
 * @copyright © 2024 Florian Staeblein
 */
 
#ifndef SYSTEMHELPER_H
#define SYSTEMHELPER_H

//===============================================================
// Inlcudes
//===============================================================
#include <Arduino.h>
#include <ESP.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <esp_log.h>
#include <esp32s2/rom/rtc.h>

//===============================================================
// Class for system handling
//===============================================================
class SystemHelper
{
  public:
    // Initializes the system helper
    void Begin();

    // Returns the complete system info as string
    String GetSystemInfoString();

    // Returns memory info string
    String GetMemoryInfoString(bool all = false);

    // Returns a string for a wifi power
    String WifiPowerToString(wifi_power_t power);

    // Returns the reset reason as string
    String GetResetReasonString(int cpu);

    // Sets the timestamp of the last user action to the current time
    void IRAM_ATTR SetLastUserAction();

    // Return the timestamp of the last user action
    uint32_t GetLastUserAction();

  private:    

    // Timestamp of last user action
    uint32_t _lastUserAction = 0;
};

//===============================================================
// Global variables
//===============================================================
extern SystemHelper Systemhelper;

#endif