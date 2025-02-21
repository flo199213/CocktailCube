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

  // Update all values
  UpdateValues();
  
  // Log startup info
  ESP_LOGI(TAG, "Finished initializing state machine");
}

//===============================================================
// Returns the angle for a given liquid
//===============================================================
int16_t StateMachine::GetAngle(MixtureLiquid liquid)
{
  switch (liquid)
  {
    case eLiquid1:
      return _liquid1Angle_Degrees;
    case eLiquid2:
      return _liquid2Angle_Degrees;
    case eLiquid3:
      return _liquid3Angle_Degrees;
    default:
      break;
  }

  return -1;
}

//===============================================================
// Returns the current mixer state of the state machine
//===============================================================
MixerState StateMachine::GetCurrentState()
{
  return _currentState;
}

//===============================================================
// General state machine execution function
//===============================================================
void StateMachine::Execute(MixerEvent event)
{
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
        // Update display and pump values
        UpdateValues();

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
            
          // Update display and pump values
          UpdateValues();

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
  switch(event)
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
        }

        // Update display and pump values
        UpdateValues();

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
                IncrementAngle(&_liquid1Angle_Degrees, _liquid2Angle_Degrees, _liquid3Angle_Degrees, currentEncoderIncrements * STEPANGLE_DEGREES);
                break;
              case eLiquid2:
                IncrementAngle(&_liquid2Angle_Degrees, _liquid3Angle_Degrees, _liquid1Angle_Degrees, currentEncoderIncrements * STEPANGLE_DEGREES);
                break;
              case eLiquid3:
                IncrementAngle(&_liquid3Angle_Degrees, _liquid1Angle_Degrees, _liquid2Angle_Degrees, currentEncoderIncrements * STEPANGLE_DEGREES);
                break;
              default:
                break;
            }
          }
          else
          {
            // Increment or decrement current setting value
            switch (_dashboardLiquid)
            {
              case eLiquid1:
                _liquid1_Percentage = max(min((int16_t)_liquid1_Percentage + currentEncoderIncrements, 95), 0);
                break;
              case eLiquid2:
                _liquid2_Percentage = max(min((int16_t)_liquid2_Percentage + currentEncoderIncrements, 95), 0);
                break;
              case eLiquid3:
                _liquid3_Percentage = max(min((int16_t)_liquid3_Percentage + currentEncoderIncrements, 95), 0);
                break;
              default:
                break;
            }
          }

          // Update display and pump values
          UpdateValues();

          if (Config.isMixer)
          {
            // Draw current value string and doughnut chart in partial updating mode
            Display.DrawCurrentValues();
            Display.DrawDoughnutChart3(currentEncoderIncrements > 0);
          }
          else
          {
            // Draw bar
            Display.DrawBar(true);
          }
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
          }

          // Update all values
          UpdateValues();
          
          if (Config.isMixer)
          {
            // Draw legend and doughnut chart in partial updating mode
            Display.DrawLegend();
            Display.DrawDoughnutChart3(false);
          }
          else
          {
            // Draw bar
            Display.DrawBar(true);
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
        // Update display and pump values
        UpdateValues();

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

          // Update display and pump values
          UpdateValues();

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
        
        // Update all values
        UpdateValues();
        
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
        // Update all values
        UpdateValues();
        
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

          // Update display and pump values
          UpdateValues();
          
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

          // Update all values
          UpdateValues();
          
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
        // Update display and pump values
        UpdateValues();

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

          // Update all values
          UpdateValues();
          
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
        // Update display and pump values
        UpdateValues();
        
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
  // Set mixture to default
  _liquid1Angle_Degrees = Config.liquid1AngleDegrees;
  _liquid2Angle_Degrees = Config.liquid2AngleDegrees;
  _liquid3Angle_Degrees = Config.liquid3AngleDegrees;
}

//===============================================================
// Updates all values in display, pumps driver and wifi
//===============================================================
void StateMachine::UpdateValues()
{
  if (Config.isMixer)
  {
    int16_t liquid1Distance_Degrees = GetDistanceDegrees(_liquid1Angle_Degrees, _liquid2Angle_Degrees);
    int16_t liquid2Distance_Degrees = GetDistanceDegrees(_liquid2Angle_Degrees, _liquid3Angle_Degrees);
    int16_t liquid3Distance_Degrees = GetDistanceDegrees(_liquid3Angle_Degrees, _liquid1Angle_Degrees);

    // Avoid minimal setable value >0%. If an angle is at its min angle, mute
    // it to zero and add the angle distance to the greater one of the two others
    if (liquid1Distance_Degrees == MINANGLE_DEGREES)
    {
      if (liquid2Distance_Degrees > liquid3Distance_Degrees)
      {
        liquid2Distance_Degrees += liquid1Distance_Degrees;
      }
      else
      {
        liquid3Distance_Degrees += liquid1Distance_Degrees;
      }
      liquid1Distance_Degrees = 0;
    }

    // Avoid minimal setable value >0%. If an angle is at its min angle, mute
    // it to zero and add the angle distance to the greater one of the two others
    if (liquid2Distance_Degrees == MINANGLE_DEGREES)
    {
      if (liquid1Distance_Degrees > liquid3Distance_Degrees)
      {
        liquid1Distance_Degrees += liquid2Distance_Degrees;
      }
      else
      {
        liquid3Distance_Degrees += liquid2Distance_Degrees;
      }
      liquid2Distance_Degrees = 0;
    }

    // Avoid minimal setable value >0%. If an angle is at its min angle, mute
    // it to zero and add the angle distance to the greater one of the two others
    if (liquid3Distance_Degrees == MINANGLE_DEGREES)
    {
      if (liquid1Distance_Degrees > liquid2Distance_Degrees)
      {
        liquid1Distance_Degrees += liquid3Distance_Degrees;
      }
      else
      {
        liquid2Distance_Degrees += liquid3Distance_Degrees;
      }
      liquid3Distance_Degrees = 0;
    }

    // Calculate percentage values
    _liquid1_Percentage = (double)liquid1Distance_Degrees * 100.0 / 360.0;
    _liquid2_Percentage = (double)liquid2Distance_Degrees * 100.0 / 360.0;
    _liquid3_Percentage = (double)liquid3Distance_Degrees * 100.0 / 360.0;
  }

  // Update display driver
  Display.SetMenuState(_currentMenuState);
  Display.SetDashboardLiquid(_dashboardLiquid);
  Display.SetCleaningLiquid(_cleaningLiquid);
  Display.SetMixerSetting(_currentSetting);
  Display.SetBar(_barBottle1, _barBottle2, _barBottle3);
  Display.SetAngles(_liquid1Angle_Degrees, _liquid2Angle_Degrees, _liquid3Angle_Degrees);
  Display.SetPercentages(_liquid1_Percentage, _liquid2_Percentage, _liquid3_Percentage);
  
  bool hasSparklingWater = _barBottle1 == eSparklingWater || _barBottle2 == eSparklingWater || _barBottle3 == eSparklingWater;
  
  // Update pump driver
  switch (_currentState)
  {
    case eDashboard:
      {
        if (Config.isMixer)
        {
          Pumps.SetPumps(_liquid1_Percentage, _liquid2_Percentage, _liquid3_Percentage);
        }
        else
        {
          if (hasSparklingWater)
          {
            switch (_dashboardLiquid)
            {
              case eLiquid1:
                {
                  double sparklingWaterPercetage = _liquid1_Percentage;
                  double winePercetage = 100.0 - sparklingWaterPercetage;
                  Pumps.SetPumps(winePercetage, _barBottle2 == eSparklingWater ? sparklingWaterPercetage : 0.0, _barBottle3 == eSparklingWater ? sparklingWaterPercetage : 0.0);
                }
                break;
              case eLiquid2:
                {
                  double sparklingWaterPercetage = _liquid2_Percentage;
                  double winePercetage = 100.0 - sparklingWaterPercetage;
                  Pumps.SetPumps(_barBottle1 == eSparklingWater ? sparklingWaterPercetage : 0.0, winePercetage, _barBottle3 == eSparklingWater ? sparklingWaterPercetage : 0.0);
                }
                break;
              case eLiquid3:
                {
                  double sparklingWaterPercetage = _liquid3_Percentage;
                  double winePercetage = 100.0 - sparklingWaterPercetage;
                  Pumps.SetPumps(_barBottle1 == eSparklingWater ? sparklingWaterPercetage : 0.0, _barBottle2 == eSparklingWater ? sparklingWaterPercetage : 0.0, winePercetage);
                }
                break;
              default:
                break;
            }
          }
        }
      }
      break;
    case eCleaning:
      {
        double cleaningLiquid1_Percentage = (_cleaningLiquid == eLiquidAll || _cleaningLiquid == eLiquid1) ? 100.0 : 0.0;
        double cleaningLiquid2_Percentage = (_cleaningLiquid == eLiquidAll || _cleaningLiquid == eLiquid2) ? 100.0 : 0.0;
        double cleaningLiquid3_Percentage = (_cleaningLiquid == eLiquidAll || _cleaningLiquid == eLiquid3) ? 100.0 : 0.0;
        Pumps.SetPumps(cleaningLiquid1_Percentage, cleaningLiquid2_Percentage, cleaningLiquid3_Percentage);
      }
      break;
    default:
    case eMenu:
    case eReset:
    case eBar:
    case eSettings:
      {
        Pumps.SetPumps(0.0, 0.0, 0.0); // zero (0%)
      }
      break;
  }
}

//===============================================================
// Returns the current mixture a string
//===============================================================
String StateMachine::GetMixtureString()
{
  // Calculate sum
  double sum_Percentage = _liquid1_Percentage + _liquid2_Percentage + _liquid3_Percentage;
  
  // Build string output
  String returnString;

  returnString += String(Config.liquid1Name) + ": " + String(_liquid1_Percentage) + "% (" + String(_liquid1Angle_Degrees) + "°), ";
  returnString += String(Config.liquid2Name) + ": " + String(_liquid2_Percentage) + "% (" + String(_liquid2Angle_Degrees) + "°), ";
  returnString += String(Config.liquid3Name) + ": " + String(_liquid3_Percentage) + "% (" + String(_liquid3Angle_Degrees) + "°), ";
  returnString += "Sum: " + String(sum_Percentage) + "%";
  
  if ((sum_Percentage - 100.0) > 0.1 || (sum_Percentage - 100.0) < -0.1)
  {
    // Percentage error
    returnString += " Error: Sum of all percentages must be ~100%";
  }

  return returnString;
}
