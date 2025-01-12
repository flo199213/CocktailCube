/**
 * Includes all state machine functions
 *
 * @author    Florian Staeblein
 * @date      2024/01/28
 * @copyright © 2024 Florian Staeblein
 */
 
#ifndef STATEMACHINE_H
#define STATEMACHINE_H

//===============================================================
// Includes
//===============================================================
#include <Arduino.h>
#include <WiFi.h>
#include <esp_log.h>
#include "Config.h"
#include "AngleHelper.h"
#include "EncoderButtonDriver.h"
#include "PumpDriver.h"
#include "DisplayDriver.h"
#include "FlowMeterDriver.h"
#include "WifiHandler.h"

//===============================================================
// Defines
//===============================================================
#define SCREENSAVER_TIMEOUT_MS      30000     // 30 seconds

//===============================================================
// Class for state machine handling
//===============================================================
class StateMachine
{
  public:
    // Constructor
    StateMachine();

    // Initializes the state machine
    void Begin(uint8_t pinBuzzer);

#if defined(WIFI_MIXER)
    // Updates cycle timespan value from wifi
    bool UpdateValuesFromWifi(uint32_t clientID, uint32_t cycletimespan_ms);

    // Updates non volatile values from wifi
    bool UpdateValuesFromWifi(uint32_t clientID, bool save);

    // Updates a liquid values from wifi
    bool UpdateValuesFromWifi(uint32_t clientID, MixtureLiquid liquid, int16_t increments_Degrees);
#endif

    // Returns the angle for a given liquid
    int16_t GetAngle(MixtureLiquid liquid);

    // Returns the current mixer state of the state machine
    MixerState IRAM_ATTR GetCurrentState();

    // General state machine execution function
    void Execute(MixerEvent event);

    // Returns the current mixture a string
    String GetMixtureString();

  private:
    // Pin definitions
    uint8_t _pinBuzzer;

    // State machine variables
    MixerState _currentState = eDashboard;
    MixerState _lastState = eDashboard;
    MixerState _currentMenuState = eDashboard;

    // Dashboard mode settings
    MixtureLiquid _dashboardLiquid = eLiquid1;
    int16_t _liquid1Angle_Degrees = 0;
    int16_t _liquid2Angle_Degrees = 0;
    int16_t _liquid3Angle_Degrees = 0;
    double _liquid1_Percentage = 0;
    double _liquid2_Percentage = 0;
    double _liquid3_Percentage = 0;

    // Cleaning mode settings
    MixtureLiquid _cleaningLiquid = eLiquidAll;

    // Timer variables for reset counter
    uint32_t _resetTimestamp = 0;
    const uint32_t ResetTime_ms = 2000;

    // Wifi new liquid data variables
    uint32_t _newLiquidClientID = 0;
    MixtureLiquid _newLiquid = eLiquidNone;
    int16_t _newLiquidIncrements_Degrees = 0;

    // Wifi new cycle timespan data variables
    uint32_t _newCycleTimespanClientID = 0;
    bool _newCycleTimespan = false;
    uint32_t _newCycleTimespan_ms = 0;

#if defined(WIFI_MIXER)
    // Handles new wifi data, should be called in state machine
    void HandleNewWifiData(MixerEvent event);
#endif

    // Function menu state
    void FctMenu(MixerEvent event);

    // Function dashboard state
    void FctDashboard(MixerEvent event);

    // Function cleaning state
    void FctCleaning(MixerEvent event);

    // Function reset state
    void FctReset(MixerEvent event);

    // Function settings state
    void FctSettings(MixerEvent event);

    // Function screen saver state
    void FctScreenSaver(MixerEvent event);
    
    // Resets the mixture to default recipe
    void SetMixtureDefaults();

    // Updates all values in display, pumps driver and wifi
    void UpdateValues(uint32_t clientID = 0);
};

//===============================================================
// Global variables
//===============================================================
extern StateMachine Statemachine;

#endif
