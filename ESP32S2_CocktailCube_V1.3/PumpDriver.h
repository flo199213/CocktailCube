/*
 * Includes all pump functions
 *
 * @author    Florian Stäblein
 * @date      2025/01/01
 * @copyright © 2025 Florian Stäblein
 */
 
#ifndef PUMPDRIVER_H
#define PUMPDRIVER_H

//===============================================================
// Includes
//===============================================================
#include <Arduino.h>
#include <esp_log.h>
#include "Config.h"
#include "StateMachine.h"
#include "FlowMeterDriver.h"

//===============================================================
// Defines
//===============================================================
#define DEFAULT_CYCLE_TIMESPAN_MS     (uint32_t)500
#define MIN_CYCLE_TIMESPAN_MS         (uint32_t)200
#define MAX_CYCLE_TIMESPAN_MS         (uint32_t)1000

#define KEY_CYCLETIMESPAN_MS          "CycleTimespan" // Key name: Maximum string length is 15 bytes, excluding a zero terminator.

//===============================================================
// Class for handling pump driver functions
//===============================================================
class PumpDriver
{
  public:
    // Constructor
    PumpDriver();

    // Initializes the pump driver
    void Begin(uint8_t pinPump1, uint8_t pinPump2, uint8_t pinPump3, double vccVoltage);
    
    // Load settings from flash
    void Load();

    // Save settings to flash
    void Save();

    // Return true, if the pumps are enabled. Otherwise false
    bool IsEnabled();

    // Sets pumps from percentage (0-100%)
    void SetPumps(double value1_Percentage, double value2_Percentage, double value3_Percentage);

    // Sets the cycle timespan in ms (200-1000ms)
    bool SetCycleTimespan(uint32_t value_ms);

    // Returns the current cycle timespan
    uint32_t GetCycleTimespan();
    
    // Should be called every < 50 ms
    void Update();
    
    // Enables or disables pump output
    void IRAM_ATTR Enable(bool enable);

  private:
    // Preferences variable
    Preferences _preferences;

    // Pin definitions
    uint8_t _pinPump1;
    uint8_t _pinPump2;
    uint8_t _pinPump3;

    // VCC voltage
    double _vccVoltage;

    // Enabled values
    bool _isPumpEnabled = false;
    bool _enablePump1 = false;
    bool _enablePump2 = false;
    bool _enablePump3 = false;

    // Timing values
    uint32_t _cycleTimespan_ms = DEFAULT_CYCLE_TIMESPAN_MS;
    uint32_t _pwmPump1_ms = 0;
    uint32_t _pwmPump2_ms = 0;
    uint32_t _pwmPump3_ms = 0;

    // Last variables for edge detection
    bool _lastIsPumpEnabled = false;
    uint32_t _lastUpdate_ms = 0;
    uint32_t _lastPumpCycleStart_ms = 0;

    // Enables pump output
    void IRAM_ATTR InternalEnable();
    
    // Disables pump output
    void IRAM_ATTR InternalDisable();
};

//===============================================================
// Global variables
//===============================================================
extern PumpDriver Pumps;

#endif
