/*
 * Includes all state machine functions
 *
 * @author    Florian Stäblein
 * @date      2025/01/01
 * @copyright © 2025 Florian Stäblein
 */

//===============================================================
// Includes
//===============================================================
#include "StateMachine.h"

//===============================================================
// Constants
//===============================================================
static const char* TAG = "statemachine";

//===============================================================
// Global variables
//===============================================================
StateMachine Statemachine;

//===============================================================
// Constructor
//===============================================================
StateMachine::StateMachine()
{
}

//===============================================================
// Initializes the state machine
//===============================================================
void StateMachine::Begin(uint8_t pinBuzzer)
{
  // Log startup info
  ESP_LOGI(TAG, "Begin initializing state machine");

  // Save pin definitions
  _pinBuzzer = pinBuzzer;

  // Set Defaults
  SetMixtureDefaults();
  
  // Log startup info
  ESP_LOGI(TAG, "Finished initializing state machine");
}

//===============================================================
// Updates cycle timespan value from wifi
//===============================================================
bool StateMachine::UpdateValuesFromWifi(uint32_t cycleTimespan_ms)
{
  // Check for min and max value
  if (cycleTimespan_ms < MIN_CYCLE_TIMESPAN_MS ||
    cycleTimespan_ms > MAX_CYCLE_TIMESPAN_MS)
  {
    return false;
  }

  // General new wifi cycle timespan data handler 
  if (Pumps.SetCycleTimespan(cycleTimespan_ms))
  {
    // Draw new values in settings mode
    if (_currentState == eSettings)
    {
      // Draw settings in partial update mode
      Display.DrawSettings();
    }

    // Update pump values
    UpdatePumpValues();
  }

  return true;
}

//===============================================================
// Updates a liquid value from wifi
//===============================================================
bool StateMachine::UpdateValuesFromWifi(MixtureLiquid liquid, int16_t increments_Degrees)
{
  // Check angle for 360 degrees increment or decrement max
  if (increments_Degrees < -360 ||
    increments_Degrees > 360)
  {
    return false;
  }

  if (Config.isMixer)
  {
    // Increment or decrement angle
    switch (liquid)
    {
      case eLiquid1:
        IncrementAngle(&_liquidAngle1, _liquidAngle2, _liquidAngle3, increments_Degrees);
        break;
      case eLiquid2:
        IncrementAngle(&_liquidAngle2, _liquidAngle3, _liquidAngle1, increments_Degrees);
        break;
      case eLiquid3:
        IncrementAngle(&_liquidAngle3, _liquidAngle1, _liquidAngle2, increments_Degrees);
        break;
      default:
        break;
    }

    // Update pump values
    UpdatePumpValues();

    // Draw new values in dashboard mode
    if (_currentState == eDashboard)
    {
      // Draw current value string and doughnut chart in partial updating mode
      Display.DrawCurrentValues();
      Display.DrawDoughnutChart3(increments_Degrees > 0);
    }
  }
  else
  {
    // Not implemented
  }

  return true;
}

//===============================================================
// Returns the current mixer state of the state machine
//===============================================================
MixerState StateMachine::GetCurrentState()
{
  return _currentState;
}

//===============================================================
// Returns the current menu state
//===============================================================
MixerState StateMachine::GetMenuState()
{
  return _currentMenuState;
}

//===============================================================
// Returns the current dashboard liquid
//===============================================================
MixtureLiquid StateMachine::GetDashboardLiquid()
{
  return _dashboardLiquid;
}

//===============================================================
// Returns the current cleaning liquid
//===============================================================
MixtureLiquid StateMachine::GetCleaningLiquid()
{
  return _cleaningLiquid;
}

//===============================================================
// Returns current mixer setting
//===============================================================
MixerSetting StateMachine::GetMixerSetting()
{
  return _currentSetting;
}

//===============================================================
// Returns the bottle of a given liquid input
//===============================================================
BarBottle StateMachine::GetBarBottle(uint8_t index)
{
  switch (index)
  {
    case 0:
      return _barBottle1;
    case 1:
      return _barBottle2;
    case 2:
      return _barBottle3;
    default:
      return eEmpty;
  }
}

//===============================================================
// Returns the angle for a given liquid (used for mixer)
//===============================================================
int16_t StateMachine::GetAngle(MixtureLiquid liquid)
{
  switch (liquid)
  {
    case eLiquid1:
      return _liquidAngle1;
    case eLiquid2:
      return _liquidAngle2;
    case eLiquid3:
      return _liquidAngle3;
    default:
      return -1;
  }
}

//===============================================================
// Returns the percentage for a given liquid (used for bar)
//===============================================================
double StateMachine::GetBarPercentage(MixtureLiquid liquid)
{
  switch (liquid)
  {
    case eLiquid1:
      return _liquidPercentage1;
    case eLiquid2:
      return _liquidPercentage2;
    case eLiquid3:
      return _liquidPercentage3;
    default:
      return -1;
  }
}

//===============================================================
    // Returns the percentage for a given pump
//===============================================================
double StateMachine::GetPumpPercentage(MixtureLiquid liquid)
{
  switch (liquid)
  {
    case eLiquid1:
      return _pumpPercentage1;
    case eLiquid2:
      return _pumpPercentage2;
    case eLiquid3:
      return _pumpPercentage3;
    default:
      return -1;
  }
}

//===============================================================
// Returns the current mixture a string
//===============================================================
String StateMachine::GetMixtureString()
{
  // Build string output
  String returnString;

  if (Config.isMixer)
  {
    // Calculate sum
    double sum_Percentage = _pumpPercentage1 + _pumpPercentage2 + _pumpPercentage3;
    
    returnString += String(Config.liquidName1) + ": " + String(_pumpPercentage1) + "% (" + String(_liquidAngle1) + "°), ";
    returnString += String(Config.liquidName2) + ": " + String(_pumpPercentage2) + "% (" + String(_liquidAngle2) + "°), ";
    returnString += String(Config.liquidName3) + ": " + String(_pumpPercentage3) + "% (" + String(_liquidAngle3) + "°), ";
    returnString += "Sum: " + String(sum_Percentage) + "%";
    
    if ((sum_Percentage - 100.0) > 0.1 || (sum_Percentage - 100.0) < -0.1)
    {
      // Percentage error
      returnString += " Error: Sum of all percentages must be ~100%";
    }
  }
  else
  {
    returnString += String(Config.liquidName1) + ": " + String(_pumpPercentage1) + "%, ";
    returnString += String(Config.liquidName2) + ": " + String(_pumpPercentage2) + "%, ";
    returnString += String(Config.liquidName3) + ": " + String(_pumpPercentage3) + "%";
  }

  return returnString;
}
    
//===============================================================
// Returns need update counter
//===============================================================
uint16_t StateMachine::GetNeedUpdate()
{
  return _needUpdate;
}

//===============================================================
// General state machine execution function
//===============================================================
void StateMachine::Execute(MixerEvent event)
{
  // If entry event update pump values before each state change
  if (event == eEntry)
  {
    UpdatePumpValues();
  }

  // Run state machine
  switch (_currentState)
  {
    case eMenu:
      FctMenu(event);
      break;
    case eCleaning:
      FctCleaning(event);
      break;
    case eReset:
      FctReset(event);
      break;
    case eSettings:
      FctSettings(event);
      break;
    case eBar:
      FctBar(event);
      break;
    case eScreenSaver:
      FctScreenSaver(event);
      break;
    default:  // In case something went wrong, default case is dashboard
    case eDashboard:
      FctDashboard(event);
      break;
  }
}

//===============================================================
// Function menu state
//===============================================================
void StateMachine::FctMenu(MixerEvent event)
{
  switch (event)
  {
    case eEntry:
      {
        // Show menu page
        ESP_LOGI(TAG, "Enter menu mode");
        Display.ShowMenuPage();

        // Debounce page change
        delay(500);

        // Reset and ignore user input
        EncoderButton.GetEncoderIncrements();
        EncoderButton.IsButtonPress();
      }
      break;
    case eMain:
      {
        // Read encoder increments (resets the counter value)
        int16_t currentEncoderIncrements = EncoderButton.GetEncoderIncrements();

        // Check for changed encoder value
        if (currentEncoderIncrements != 0)
        {
          // Increment or decrement current menu state
          switch (_currentMenuState)
          {
            case eDashboard:
              _currentMenuState = currentEncoderIncrements > 0 ? eDashboard : eCleaning;
              break;
            case eCleaning:
              _currentMenuState = currentEncoderIncrements > 0 ? eDashboard : (Config.isMixer ? eReset : eBar);
              break;
            case eReset:
              _currentMenuState = currentEncoderIncrements > 0 ? eCleaning : eSettings;
              break;
            case eBar:
              _currentMenuState = currentEncoderIncrements > 0 ? eCleaning : eSettings;
              break;
            case eSettings:
              _currentMenuState = currentEncoderIncrements > 0 ? (Config.isMixer ? eReset : eBar) : eSettings;
              break;
            default:
              break;
          }

          // Update menu
          Display.DrawMenu();
        }

        // Draw wifi icons
        Display.DrawWifiIcons();

        // Check for button press
        if (EncoderButton.IsButtonPress())
        {
          // Short beep sound
          tone(_pinBuzzer, 500, 40);

          // Exit menu and enter new selected mode
          Execute(eExit);
          _currentState = _currentMenuState;
          Execute(eEntry);
          return;
        }

        // Check for screen saver timeout
        if (millis() - Systemhelper.GetLastUserAction() > SCREENSAVER_TIMEOUT_MS)
        {
          // Exit menu mode and enter screen saver mode
          Execute(eExit);
          _lastState = eMenu;
          _currentState = eScreenSaver;
          Execute(eEntry);
          return;
        }
      }
      break;
    case eExit:
    default:
      break;
  }
}

//===============================================================
// Function dashboard state
//===============================================================
void StateMachine::FctDashboard(MixerEvent event)
{
  switch (event)
  {
    case eEntry:
      {
        if (!Config.isMixer)
        {
          // If all three bottles are configured empty thats okay, because in this case the checkboxes will be displayed
          bool allEmpty = _barBottle1 == eEmpty && _barBottle2 == eEmpty && _barBottle3 == eEmpty;

          int16_t secureCounter = 0;
          switch (_dashboardLiquid)
          {
            case eLiquid1:
              if (_barBottle1 == eEmpty && !allEmpty)
              {
                // Skip empty bottle settings
                while (secureCounter++ < 3)
                {
                  // Incrementing the setting value taking into account the overflow
                  _dashboardLiquid = _dashboardLiquid + 1 >= (MixtureLiquid)MixtureLiquidDashboardMax ? eLiquid1 : (MixtureLiquid)(_dashboardLiquid + 1);

                  // Break only if current bottle is not empty
                  switch (_dashboardLiquid)
                  {
                    case eLiquid1:
                      if (_barBottle1 != eEmpty) { secureCounter = 3; }
                      break;
                    case eLiquid2:
                      if (_barBottle2 != eEmpty) { secureCounter = 3; }
                      break;
                    case eLiquid3:
                      if (_barBottle3 != eEmpty) { secureCounter = 3; }
                      break;
                    default:
                      secureCounter = 3;
                      break;
                  }
                }
              }
              break;
            case eLiquid2:
              if (_barBottle2 == eEmpty && !allEmpty)
              {
                // Skip empty bottle settings
                while (secureCounter++ < 3)
                {
                  // Incrementing the setting value taking into account the overflow
                  _dashboardLiquid = _dashboardLiquid + 1 >= (MixtureLiquid)MixtureLiquidDashboardMax ? eLiquid1 : (MixtureLiquid)(_dashboardLiquid + 1);
                  
                  // Break only if current bottle is not empty
                  switch (_dashboardLiquid)
                  {
                    case eLiquid1:
                      if (_barBottle1 != eEmpty) { secureCounter = 3; }
                      break;
                    case eLiquid2:
                      if (_barBottle2 != eEmpty) { secureCounter = 3; }
                      break;
                    case eLiquid3:
                      if (_barBottle3 != eEmpty) { secureCounter = 3; }
                      break;
                    default:
                      secureCounter = 3;
                      break;
                  }
                }
              }
              break;
            case eLiquid3:
              if (_barBottle3 == eEmpty && !allEmpty)
              {
                // Skip empty bottle settings
                while (secureCounter++ < 3)
                {
                  // Incrementing the setting value taking into account the overflow
                  _dashboardLiquid = _dashboardLiquid + 1 >= (MixtureLiquid)MixtureLiquidDashboardMax ? eLiquid1 : (MixtureLiquid)(_dashboardLiquid + 1);
                  
                  // Break only if current bottle is not empty
                  switch (_dashboardLiquid)
                  {
                    case eLiquid1:
                      if (_barBottle1 != eEmpty) { secureCounter = 3; }
                      break;
                    case eLiquid2:
                      if (_barBottle2 != eEmpty) { secureCounter = 3; }
                      break;
                    case eLiquid3:
                      if (_barBottle3 != eEmpty) { secureCounter = 3; }
                      break;
                    default:
                      secureCounter = 3;
                      break;
                  }
                }
              }
              break;
            default:
              break;
          }

          // Update pump values
          UpdatePumpValues();
        }

        // Show dashboard page
        ESP_LOGI(TAG, "Enter dashboard mode");
        Display.ShowDashboardPage();

        // Debounce page change
        delay(500);

        // Reset and ignore user input
        EncoderButton.GetEncoderIncrements();
        EncoderButton.IsLongButtonPress();
        EncoderButton.IsButtonPress();
      }
      break;
    case eMain:
      {
        // Read encoder increments (resets the counter value)
        int16_t currentEncoderIncrements = EncoderButton.GetEncoderIncrements();

        // Will be true, if new encoder position is available
        if (currentEncoderIncrements != 0)
        {
          if (Config.isMixer)
          {
            // Increment or decrement angle
            switch (_dashboardLiquid)
            {
              case eLiquid1:
                IncrementAngle(&_liquidAngle1, _liquidAngle2, _liquidAngle3, currentEncoderIncrements * STEPANGLE_DEGREES);
                break;
              case eLiquid2:
                IncrementAngle(&_liquidAngle2, _liquidAngle3, _liquidAngle1, currentEncoderIncrements * STEPANGLE_DEGREES);
                break;
              case eLiquid3:
                IncrementAngle(&_liquidAngle3, _liquidAngle1, _liquidAngle2, currentEncoderIncrements * STEPANGLE_DEGREES);
                break;
              default:
                break;
            }
            
            // Draw current value string and doughnut chart in partial updating mode
            Display.DrawCurrentValues();
            Display.DrawDoughnutChart3(currentEncoderIncrements > 0);
          }
          else
          {
            // Increment or decrement current setting value
            switch (_dashboardLiquid)
            {
              case eLiquid1:
                _liquidPercentage1 = max(min((int16_t)_liquidPercentage1 + currentEncoderIncrements, 95), 0);
                break;
              case eLiquid2:
                _liquidPercentage2 = max(min((int16_t)_liquidPercentage2 + currentEncoderIncrements, 95), 0);
                break;
              case eLiquid3:
                _liquidPercentage3 = max(min((int16_t)_liquidPercentage3 + currentEncoderIncrements, 95), 0);
                break;
              default:
                break;
            }

            // Draw bar
            Display.DrawBar(true);
          }

          // Update pump values
          UpdatePumpValues();
        }

        // Check for button press
        if (EncoderButton.IsButtonPress())
        {
          // Short beep sound
          tone(_pinBuzzer, 500, 40);

          if (Config.isMixer)
          {
            // Incrementing the setting value taking into account the overflow
            _dashboardLiquid = _dashboardLiquid + 1 >= (MixtureLiquid)MixtureLiquidDashboardMax ? eLiquid1 : (MixtureLiquid)(_dashboardLiquid + 1);

            // Draw legend and doughnut chart in partial updating mode
            Display.DrawLegend();
            Display.DrawDoughnutChart3(false);
          }
          else
          {
            // Skip empty bottle settings
            int16_t secureCounter = 0;
            do
            {
              // Incrementing the setting value taking into account the overflow
              _dashboardLiquid = _dashboardLiquid + 1 >= (MixtureLiquid)MixtureLiquidDashboardMax ? eLiquid1 : (MixtureLiquid)(_dashboardLiquid + 1);

              // If all three bottles are configured empty thats okay, because in this case the checkboxes will be displayed
              if (_barBottle1 == eEmpty && _barBottle2 == eEmpty && _barBottle3 == eEmpty)
              {
                break;
              }

              // Break only if current bottle is not empty
              switch (_dashboardLiquid)
              {
                case eLiquid1:
                  if (_barBottle1 != eEmpty) { secureCounter = 3; }
                  break;
                case eLiquid2:
                  if (_barBottle2 != eEmpty) { secureCounter = 3; }
                  break;
                case eLiquid3:
                  if (_barBottle3 != eEmpty) { secureCounter = 3; }
                  break;
                default:
                  secureCounter = 3;
                  break;
              }
            }
            while (secureCounter++ < 3);

            // Draw bar
            Display.DrawBar(true);
          
            // Update pump values
            UpdatePumpValues();
          }

          // Debounce settings change
          delay(200);
        }

        // Draw wifi icons
        Display.DrawWifiIcons();
        
        // Check for long button press
        if (EncoderButton.IsLongButtonPress())
        {
          // Short beep sound
          tone(_pinBuzzer, 800, 40);

          // Exit dashboard and enter menu mode
          Execute(eExit);
          _currentState = eMenu;
          _currentMenuState = eDashboard;
          Execute(eEntry);
          return;
        }

        // Check for screen saver timeout
        if (millis() - Systemhelper.GetLastUserAction() > SCREENSAVER_TIMEOUT_MS)
        {
          // Exit dashboard mode and enter screen saver mode
          Execute(eExit);
          _lastState = eDashboard;
          _currentState = eScreenSaver;
          Execute(eEntry);
          return;
        }
      }
      break;
    case eExit:
    default:
      break;
  }
}

//===============================================================
// Function cleaning state
//===============================================================
void StateMachine::FctCleaning(MixerEvent event)
{
  switch(event)
  {
    case eEntry:
      {
        // Show cleaning page
        ESP_LOGI(TAG, "Enter cleaning mode");
        Display.ShowCleaningPage();

        // Debounce page change
        delay(500);

        // Reset and ignore user input
        EncoderButton.IsButtonPress();
        EncoderButton.IsLongButtonPress();
      }
      break;
    case eMain:
      {
        // Check for button press
        if (EncoderButton.IsButtonPress())
        {
          // Short beep sound
          tone(_pinBuzzer, 500, 40);
          
          // Incrementing the setting value taking into account the overflow
          _cleaningLiquid = _cleaningLiquid + 1 >= (MixtureLiquid)MixtureLiquidCleaningMax ? eLiquid1 : (MixtureLiquid)(_cleaningLiquid + 1);

          // Update pump values
          UpdatePumpValues();

          // Draw checkboxes
          Display.DrawCheckBoxes(_cleaningLiquid);

          // Debounce settings change
          delay(200);
        }
        
        // Draw wifi icons
        Display.DrawWifiIcons();
        
        // Check for long button press
        if (EncoderButton.IsLongButtonPress())
        {
          // Short beep sound
          tone(_pinBuzzer, 800, 40);

          // Exit cleaning mode and return to menu mode
          Execute(eExit);
          _currentState = eMenu;
          _currentMenuState = eCleaning;
          Execute(eEntry);
          return;
        }
        
        // Check for screen saver timeout
        if (millis() - Systemhelper.GetLastUserAction() > SCREENSAVER_TIMEOUT_MS)
        {
          // Exit cleaning mode and enter screen saver mode
          Execute(eExit);
          _lastState = eCleaning;
          _currentState = eScreenSaver;
          Execute(eEntry);
          return;
        }
      }
      break;
    case eExit:
    default:
      break;
  }
}

//===============================================================
// Function reset state
//===============================================================
void StateMachine::FctReset(MixerEvent event)
{
  switch(event)
  {
    case eEntry:
      {
        // Reset mixture
        SetMixtureDefaults();

        // Draw reset info box over current page
        ESP_LOGI(TAG, "Enter reset mode");
        Display.DrawInfoBox("Mixture", "reset!");

        // Save reset page start time
        _resetTimestamp = millis();

        // Long beep sound
        tone(_pinBuzzer, 800, 500);
      }
      break;
    case eMain:
      {        
        // Wait for the reset page display time
        if ((millis() - _resetTimestamp) > ResetTime_ms)
        {
          // Exit reset mode and return to dashboard mode
          Execute(eExit);
          _currentState = eDashboard;
          Execute(eEntry);
          return;
        }
      }
      break;
    case eExit:
    default:
      break;
  }
}


//===============================================================
// Function bar state
//===============================================================
void StateMachine::FctBar(MixerEvent event)
{
  switch(event)
  {
    case eEntry:
      {        
        // Show bar page
        ESP_LOGI(TAG, "Enter Bar mode");
        Display.ShowBarPage();

        // Debounce page change
        delay(500);

        // Reset and ignore user input
        EncoderButton.GetEncoderIncrements();
        EncoderButton.IsButtonPress();
        EncoderButton.IsLongButtonPress();
      }
      break;
    case eMain:
      {
        // Read encoder increments (resets the counter value)
        int16_t currentEncoderIncrements = EncoderButton.GetEncoderIncrements();

        // Will be true, if new encoder position is available
        if (currentEncoderIncrements != 0)
        {
          // Increment or decrement current setting value
          switch (_dashboardLiquid)
          {
            case eLiquid1:
              {
                bool hasAlreadySparklingWater = _barBottle2 == eSparklingWater || _barBottle3 == eSparklingWater;
                if (currentEncoderIncrements > 0)
                {
                  _barBottle1 = _barBottle1 + 1 >= (BarBottle)BarBottleMax ? (hasAlreadySparklingWater ? eEmpty : eSparklingWater) : (BarBottle)(_barBottle1 + 1);
                }
                else
                {
                  _barBottle1 = _barBottle1 - 1 < (hasAlreadySparklingWater ? eEmpty : eSparklingWater) ? (BarBottle)(BarBottleMax - 1) : (BarBottle)(_barBottle1 - 1);
                }
              }
              break;
            case eLiquid2:
              {
                bool hasAlreadySparklingWater = _barBottle1 == eSparklingWater || _barBottle3 == eSparklingWater;
                if (currentEncoderIncrements > 0)
                {
                  _barBottle2 = _barBottle2 + 1 >= (BarBottle)BarBottleMax ? (hasAlreadySparklingWater ? eEmpty : eSparklingWater) : (BarBottle)(_barBottle2 + 1);
                }
                else
                {
                  _barBottle2 = _barBottle2 - 1 < (hasAlreadySparklingWater ? eEmpty : eSparklingWater) ? (BarBottle)(BarBottleMax - 1) : (BarBottle)(_barBottle2 - 1);
                }
              }
              break;
            case eLiquid3:
              {
                bool hasAlreadySparklingWater = _barBottle1 == eSparklingWater || _barBottle2 == eSparklingWater;
                if (currentEncoderIncrements > 0)
                {
                  _barBottle3 = _barBottle3 + 1 >= (BarBottle)BarBottleMax ? (hasAlreadySparklingWater ? eEmpty : eSparklingWater) : (BarBottle)(_barBottle3 + 1);
                }
                else
                {
                  _barBottle3 = _barBottle3 - 1 < (hasAlreadySparklingWater ? eEmpty : eSparklingWater) ? (BarBottle)(BarBottleMax - 1) : (BarBottle)(_barBottle3 - 1);
                }
              }
              break;
            default:
              break;
          }

          // Short beep sound
          tone(_pinBuzzer, 500, 40);
          
          // Draw bar
          Display.DrawBar(false);
          
          // Debounce settings change
          delay(200);
        }
        
        // Draw wifi icons
        Display.DrawWifiIcons();
        
        // Check for button press
        if (EncoderButton.IsButtonPress())
        {
          // Short beep sound
          tone(_pinBuzzer, 500, 40);

          // Incrementing the setting value taking into account the overflow
          _dashboardLiquid = _dashboardLiquid + 1 >= (MixtureLiquid)MixtureLiquidDashboardMax ? eLiquid1 : (MixtureLiquid)(_dashboardLiquid + 1);
          
          // Draw bar
          Display.DrawBar(false);
          
          // Debounce settings change
          delay(200);
        }

        // Check for long button press
        if (EncoderButton.IsLongButtonPress())
        {
          // Short beep sound
          tone(_pinBuzzer, 800, 40);

          // Exit bar mode and return to menu mode
          Execute(eExit);
          _currentState = eMenu;
          _currentMenuState = eBar;
          Execute(eEntry);
          return;
        }

         // Check for screen saver timeout
        if (millis() - Systemhelper.GetLastUserAction() > SCREENSAVER_TIMEOUT_MS)
        {
          // Exit bar mode and enter screen saver mode
          Execute(eExit);
          _lastState = eBar;
          _currentState = eScreenSaver;
          Execute(eEntry);
          return;
        }
      }
      break;
    case eExit:
    default:
      break;
  }
}

//===============================================================
// Function settings state
//===============================================================
void StateMachine::FctSettings(MixerEvent event)
{
  switch(event)
  {
    case eEntry:
      {
        // Update configurations
        Config.EnumerateConfigs();

        // Show settings page
        ESP_LOGI(TAG, "Enter settings mode");
        Display.ShowSettingsPage();
        
        // Debounce page change
        delay(500);

        // Reset and ignore user input
        EncoderButton.GetEncoderIncrements();
        EncoderButton.IsLongButtonPress(); 
        EncoderButton.IsButtonPress();
      }
      break;
    case eMain:
      {
        // Read encoder increments (resets the counter value)
        int16_t currentEncoderIncrements = EncoderButton.GetEncoderIncrements();

        // Will be true, if new encoder position is available
        if (currentEncoderIncrements != 0)
        {
          switch (_currentSetting)
          {
            case ePWM:
              // Update cycle timespan
              Pumps.SetCycleTimespan(Pumps.GetCycleTimespan() + currentEncoderIncrements * 20);
              break;

            case eWLAN:
              // Update wifi mode
              Wifihandler.SetWifiMode(Wifihandler.GetWifiMode() == WIFI_MODE_AP ? WIFI_MODE_NULL : WIFI_MODE_AP);
              Display.DrawWifiIcons(true);
              break;

            case eConfig:
              // Update configuration
              if ((currentEncoderIncrements > 0 && Config.Increment()) ||
                (currentEncoderIncrements < 0 && Config.Decrement()))
              {
                // Load new configuration
                if (Config.LoadConfig(Config.GetCurrent()))
                {
                  // Load new images
                  Display.LoadImages();

                  // Show intro page for a few milliseconds
                  Display.ShowIntroPage();
                  delay(800);
                }
                else
                {
                  // Reset config if loading failed
                  Config.ResetConfig();
                }
                
                // Increment need update counter fpr wifi clients
                _needUpdate++;

                // Reset mixture
                SetMixtureDefaults();
                  
                // Reset to settings page
                Display.ShowSettingsPage();
              }
              break;
          }

          // Draw settings in partial update mode
          Display.DrawSettings();
          
          // Discard encoder increments
          EncoderButton.GetEncoderIncrements();
        }
        
        // Check for short button press
        if (EncoderButton.IsButtonPress())
        {
          // Short beep sound
          tone(_pinBuzzer, 500, 40);

          // Incrementing the setting value taking into account the overflow
          _currentSetting = (uint16_t)_currentSetting + 1 >= MixerSettingMax ? ePWM : (MixerSetting)(_currentSetting + 1);
          
          // Draw settings
          Display.DrawSettings();
          
          // Debounce settings change
          delay(200);
        }

        // Draw wifi icons
        Display.DrawWifiIcons();

        // Check for long button press
        if (EncoderButton.IsLongButtonPress())
        {
          // Short beep sound
          tone(_pinBuzzer, 800, 40);

          // Exit cleaning mode and return to menu mode
          Execute(eExit);
          _currentState = eMenu;
          _currentMenuState = eSettings;
          Execute(eEntry);
          return;
        }

        // Check for screen saver timeout
        if (millis() - Systemhelper.GetLastUserAction() > SCREENSAVER_TIMEOUT_MS)
        {
          // Exit settings mode and enter screen saver mode
          Execute(eExit);
          _lastState = eSettings;
          _currentState = eScreenSaver;
          Execute(eEntry);
          return;
        }
      }
      break;
    case eExit:
      {
        Pumps.Save();
        Wifihandler.Save();
        Config.Save();
      }
      break;
    default:
      break;
  }
}

//===============================================================
// Function screen saver state
//===============================================================
void StateMachine::FctScreenSaver(MixerEvent event)
{
  switch(event)
  {
    case eEntry:
      {        
        // Show screen saver page
        ESP_LOGI(TAG, "Enter screen saver mode");
        Display.ShowScreenSaverPage();
        
        // Reset and ignore user input
        EncoderButton.GetEncoderIncrements();
        EncoderButton.IsLongButtonPress(); 
        EncoderButton.IsButtonPress();
      }
      break;
    case eMain:
      {
        // Draw screen saver
        Display.DrawScreenSaver();

        // Check user input (Last user interaction will be set)
        EncoderButton.GetEncoderIncrements();
        EncoderButton.IsLongButtonPress(); 
        EncoderButton.IsButtonPress();
        
        // Check for last user interaction
        if (millis() - Systemhelper.GetLastUserAction() <= SCREENSAVER_TIMEOUT_MS)
        {
          // Exit screen saver mode and return to last mode
          Execute(eExit);
          _currentState = _lastState;
          Execute(eEntry);
          return;
        }
      }
      break;
    case eExit:
    default:
      break;
  }
}

//===============================================================
// Resets the mixture to default recipe
//===============================================================
void StateMachine::SetMixtureDefaults()
{
  if (Config.isMixer)
  {
    // Set mixture to default
    _liquidAngle1 = Config.liquidAngle1;
    _liquidAngle2 = Config.liquidAngle2;
    _liquidAngle3 = Config.liquidAngle3;
  }
  else
  {
    // Reset bar mode percentages
    _liquidPercentage1 = 0;
    _liquidPercentage2 = 0;
    _liquidPercentage3 = 0;
  }

  // Update pump values
  UpdatePumpValues();
}

//===============================================================
// Updates pump driver values
//===============================================================
void StateMachine::UpdatePumpValues()
{
  // Angles to percentages (mixer mode)
  if (Config.isMixer)
  {
    int16_t liquid1Distance_Degrees = GetDistanceDegrees(_liquidAngle1, _liquidAngle2);
    int16_t liquid2Distance_Degrees = GetDistanceDegrees(_liquidAngle2, _liquidAngle3);
    int16_t liquid3Distance_Degrees = GetDistanceDegrees(_liquidAngle3, _liquidAngle1);

    // Mute angle if below min angle
    MuteMinAngle(&liquid1Distance_Degrees, &liquid2Distance_Degrees, &liquid3Distance_Degrees);
    MuteMinAngle(&liquid2Distance_Degrees, &liquid1Distance_Degrees, &liquid3Distance_Degrees);
    MuteMinAngle(&liquid3Distance_Degrees, &liquid1Distance_Degrees, &liquid2Distance_Degrees);

    // Calculate pump percentage values
    _pumpPercentage1 = (double)liquid1Distance_Degrees * 100.0 / 360.0;
    _pumpPercentage2 = (double)liquid2Distance_Degrees * 100.0 / 360.0;
    _pumpPercentage3 = (double)liquid3Distance_Degrees * 100.0 / 360.0;
  }
  // Percentages to percentages (bar mode)
  else
  {
    // Check for any sparkling water connected
    if (_barBottle1 == eSparklingWater ||
      _barBottle2 == eSparklingWater ||
      _barBottle3 == eSparklingWater)
    {
      switch (_dashboardLiquid)
      {
        case eLiquid1:
          {
            _pumpPercentage1 = _barBottle1 == eSparklingWater ? 100.0 : 100.0 - _liquidPercentage1;
            _pumpPercentage2 = _barBottle2 == eSparklingWater ? _liquidPercentage1 : 0.0;
            _pumpPercentage3 = _barBottle3 == eSparklingWater ? _liquidPercentage1 : 0.0;
          }
          break;
        case eLiquid2:
          {
            _pumpPercentage1 = _barBottle1 == eSparklingWater ? _liquidPercentage2 : 0.0;
            _pumpPercentage2 = _barBottle2 == eSparklingWater ? 100.0 : 100.0 - _liquidPercentage2;
            _pumpPercentage3 = _barBottle3 == eSparklingWater ? _liquidPercentage2 : 0.0;
          }
          break;
        case eLiquid3:
          {
            _pumpPercentage1 = _barBottle1 == eSparklingWater ? _liquidPercentage3 : 0.0;
            _pumpPercentage2 = _barBottle2 == eSparklingWater ? _liquidPercentage3 : 0.0;
            _pumpPercentage3 = _barBottle3 == eSparklingWater ? 100.0 : 100.0 - _liquidPercentage3;
          }
          break;
        default:
          break;
      }
    }
    else
    {
      _pumpPercentage1 = _dashboardLiquid == eLiquid1 ? 100.0 : 0.0;
      _pumpPercentage2 = _dashboardLiquid == eLiquid2 ? 100.0 : 0.0;
      _pumpPercentage3 = _dashboardLiquid == eLiquid3 ? 100.0 : 0.0;
    }
  }

  // Default is zero
  double pumpPercentage1 = 0.0;
  double pumpPercentage2 = 0.0;
  double pumpPercentage3 = 0.0;

  // Update pump driver depending on mixer state
  switch (_currentState)
  {
    case eDashboard:
      // Set precalculated pump percentage values
      pumpPercentage1 = _pumpPercentage1;
      pumpPercentage2 = _pumpPercentage2;
      pumpPercentage3 = _pumpPercentage3;
      break;
    case eCleaning:
      // Set cleaning pumps to 100%
      pumpPercentage1 = (_cleaningLiquid == eLiquidAll || _cleaningLiquid == eLiquid1) ? 100.0 : 0.0;
      pumpPercentage2 = (_cleaningLiquid == eLiquidAll || _cleaningLiquid == eLiquid2) ? 100.0 : 0.0;
      pumpPercentage3 = (_cleaningLiquid == eLiquidAll || _cleaningLiquid == eLiquid3) ? 100.0 : 0.0;
      break;
    default:
    case eMenu:
    case eReset:
    case eBar:
    case eSettings:
      // Leave pumps at zero (0%)
      break;
  }

  // Set values in pumps driver
  Pumps.SetPumps(pumpPercentage1, pumpPercentage2, pumpPercentage3);
}

//===============================================================
// Sets the angle to zero if below min angle and adds the value 
// to the bigger other angle
//===============================================================
void StateMachine::MuteMinAngle(int16_t* angleToMute, int16_t* otherAngle1, int16_t* otherAngle2)
{
  // Avoid that the minimal setable angle value is bigger than 0% -> If an
  // angle is at its min angle, mute it to zero and add the angle distance
  // to the greater one of the two others
  if (*angleToMute <= MINANGLE_DEGREES)
  {
    if (*otherAngle1 > *otherAngle2)
    {
      *otherAngle1 += *angleToMute;
    }
    else
    {
      *otherAngle2 += *angleToMute;
    }
    *angleToMute = 0;
  }
}
