/*
 * Includes all flow meter functions
 *
 * @author    Florian Stäblein
 * @date      2025/01/01
 * @copyright © 2025 Florian Stäblein
 */
 
#ifndef FLOWMETERDRIVER_H
#define FLOWMETERDRIVER_H

//===============================================================
// Includes
//===============================================================
#include <Arduino.h>
#include <Preferences.h>
#include <esp_log.h>
#include "Config.h"

//===============================================================
// Defines
//===============================================================
#define FLOWRATE1             0.00000416667   // 250 ml/min (pump 1 specification @ 24V) => 5e-6 l/ms
#define FLOWRATE2             0.00000416667   // 250 ml/min (pump 2 specification @ 24V) => 5e-6 l/ms
#define FLOWRATE3             0.00000416667   // 250 ml/min (pump 3 specification @ 24V) => 5e-6 l/ms

#define KEY_FLOW_LIQUID1      "FlowLiquid1"   // Key name: Maximum string length is 15 bytes, excluding a zero terminator.
#define KEY_FLOW_LIQUID2      "FlowLiquid2"   // Key name: Maximum string length is 15 bytes, excluding a zero terminator.
#define KEY_FLOW_LIQUID3      "FlowLiquid3"   // Key name: Maximum string length is 15 bytes, excluding a zero terminator.

//===============================================================
// Class for flow measuring
//===============================================================
class FlowMeterDriver
{
  public:
    // Constructor
    FlowMeterDriver();
    
    // Initializes the flow meter driver
    void Begin();
    
    // Load settings from flash
    void Load();

    // Save settings to flash
    void Save();

    // Returns current flow meter values
    double GetValueLiquid1();
    double GetValueLiquid2();
    double GetValueLiquid3();

    // Adds flow time (@100% pump power) to flow meter
    void AddFlowTime(uint32_t valueLiquid1_ms, uint32_t valueLiquid2_ms, uint32_t valueLiquid3_ms);
    
  private:
    // Flow meter variables
    Preferences _preferences;

    // Save variable
    bool _savePending = false;

    // Liquid counter variables
    double _valueLiquid1_L;
    double _valueLiquid2_L;
    double _valueLiquid3_L;
};

//===============================================================
// Global variables
//===============================================================
extern FlowMeterDriver FlowMeter;

#endif
