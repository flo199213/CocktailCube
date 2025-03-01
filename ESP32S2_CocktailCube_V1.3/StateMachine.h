/*
 * Includes all state machine functions
 *
 * @author    Florian Stäblein
 * @date      2025/01/01
 * @copyright © 2025 Florian Stäblein
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
// Class for state machine handling
//===============================================================
class StateMachine
{
  public:
    // Constructor
    StateMachine();

    // Initializes the state machine
    void Begin(uint8_t pinBuzzer);

    // Updates cycle timespan value from wifi
    bool UpdateValuesFromWifi(uint32_t cycletimespan_ms);

    // Updates a liquid values from wifi
    bool UpdateValuesFromWifi(MixtureLiquid liquid, int16_t increments_Degrees);

    // Returns the current mixer state of the state machine
    MixerState IRAM_ATTR GetCurrentState();
    
    // Returns the current menu state
    MixerState GetMenuState();

    // Returns the current dashboard liquid
    MixtureLiquid GetDashboardLiquid();

    // Returns the current cleaning liquid
    MixtureLiquid GetCleaningLiquid();

    // Returns current mixer setting
    MixerSetting GetMixerSetting();

    // Returns true, if the setting is selected
    bool GetSettingSelected();

    // Returns the bottle of a given liquid input
    BarBottle GetBarBottle(uint8_t index);

    // Returns the angle for a given liquid (used for mixer)
    int16_t GetAngle(MixtureLiquid liquid);

    // Returns the percentage for a given liquid (used for bar)
    double GetBarPercentage(MixtureLiquid liquid);
    
    // Returns the percentage for a given pump
    double GetPumpPercentage(MixtureLiquid liquid);

    // Returns the current mixture a string
    String GetMixtureString();
    
    // Returns need update counter
    uint16_t GetNeedUpdate();

    // General state machine execution function
    void Execute(MixerEvent event);

  private:
    // Pin definitions
    uint8_t _pinBuzzer;

    // State machine variables
    MixerState _currentState = eDashboard;
    MixerState _lastState = eDashboard;
    MixerState _currentMenuState = eDashboard;

    // Dashboard mode settings
    MixtureLiquid _dashboardLiquid = eLiquid1;
    int16_t _liquidAngle1 = 0;      // Setvalue 1 for mixer mode
    int16_t _liquidAngle2 = 0;      // Setvalue 2 for mixer mode
    int16_t _liquidAngle3 = 0;      // Setvalue 3 for mixer mode
    double _liquidPercentage1 = 0;  // Setvalue 1 for bar mode
    double _liquidPercentage2 = 0;  // Setvalue 2 for bar mode
    double _liquidPercentage3 = 0;  // Setvalue 3 for bar mode

    // Precalculated pump values
    double _pumpPercentage1 = 0;
    double _pumpPercentage2 = 0;
    double _pumpPercentage3 = 0;

    // Cleaning mode settings
    MixtureLiquid _cleaningLiquid = eLiquidAll;

    // Bar settings
    BarBottle _barBottle1 = eRedWine;
    BarBottle _barBottle2 = eWhiteWine;
    BarBottle _barBottle3 = eRoseWine;

    // Setting mode settings
    MixerSetting _currentSetting = ePWM;
    bool _settingSelected = false;

    // Timer variables for reset counter
    uint32_t _resetTimestamp = 0;
    const uint32_t ResetTime_ms = 2000;

    // Need Update for wifi
    uint16_t _needUpdate = 0;

    // Function menu state
    void FctMenu(MixerEvent event);

    // Function dashboard state
    void FctDashboard(MixerEvent event);

    // Function cleaning state
    void FctCleaning(MixerEvent event);

    // Function bar state
    void FctBar(MixerEvent event);
    
    // Function reset state
    void FctReset(MixerEvent event);

    // Function settings state
    void FctSettings(MixerEvent event);

    // Function screen saver state
    void FctScreenSaver(MixerEvent event);
    
    // Resets the mixture to default recipe
    void SetMixtureDefaults();

    // Updates pump driver values
    void UpdatePumpValues();
    
    // Sets the angle to zero if below min angle and adds the value  to the bigger other angle
    void MuteMinAngle(int16_t* angleToMute, int16_t* otherAngle1, int16_t* otherAngle2);
};

//===============================================================
// Global variables
//===============================================================
extern StateMachine Statemachine;

#endif
