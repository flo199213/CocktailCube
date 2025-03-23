/*
 * Includes pump driver functions
 *
 * @author    Florian Stäblein
 * @date      2025/01/01
 * @copyright © 2025 Florian Stäblein
 */

//===============================================================
// Includes
//===============================================================
#include "PumpDriver.h"

//===============================================================
// Constants
//===============================================================
static const char* TAG = "pumps";

//===============================================================
// Global variables
//===============================================================
PumpDriver Pumps;

//===============================================================
// Constructor
//===============================================================
PumpDriver::PumpDriver()
{
}

//===============================================================
// Initializes the pump driver
//===============================================================
void PumpDriver::Begin(uint8_t pinPump1, uint8_t pinPump2, uint8_t pinPump3, double vccVoltage)
{
  // Log startup info
  ESP_LOGI(TAG, "Begin initializing pump driver");

  // Set pins
  _pinPump1 = pinPump1;
  _pinPump2 = pinPump2;
  _pinPump3 = pinPump3;

  // Save VCC voltage
  _vccVoltage = vccVoltage;

  // Load settings
  Load();

  // Disable pump output
  InternalDisable();
  
  // Log startup info
  ESP_LOGI(TAG, "Finished initializing pump driver");
}

//===============================================================
// Load settings from flash
//===============================================================
void PumpDriver::Load()
{
  if (_preferences.begin(SETTINGS_NAME, READONLY_MODE))
  {
    _cycleTimespan_ms = _preferences.getLong(KEY_CYCLETIMESPAN_MS, DEFAULT_CYCLE_TIMESPAN_MS);

    ESP_LOGI(TAG, "Preferences successfully loaded from '%s'", SETTINGS_NAME);
  }
  else
  {
    ESP_LOGE(TAG, "Could not open preferences '%s'", SETTINGS_NAME);
  }

  _preferences.end();
}

//===============================================================
// Save settings to flash
//===============================================================
void PumpDriver::Save()
{
  if (_preferences.begin(SETTINGS_NAME, READWRITE_MODE))
  {
    _preferences.putLong(KEY_CYCLETIMESPAN_MS, _cycleTimespan_ms);

    ESP_LOGI(TAG, "Preferences successfully saved to '%s'", SETTINGS_NAME);
  }
  else
  {
    ESP_LOGE(TAG, "Could not open preferences '%s'", SETTINGS_NAME);
  }

  _preferences.end();
}

//===============================================================
// Return true, if the pumps are enabled. Otherwise false
//===============================================================
bool PumpDriver::IsEnabled()
{
  return _isPumpEnabled;
}

//===============================================================
// Sets pumps from percentage (0-100%)
//===============================================================
void PumpDriver::SetPumps(double value1_Percentage, double value2_Percentage, double value3_Percentage)
{
  // Check min and max borders (0-100%)
  double pump1Clip_Percentage = min(max(value1_Percentage, 0.0), 100.0);
  double pump2Clip_Percentage = min(max(value2_Percentage, 0.0), 100.0);
  double pump3Clip_Percentage = min(max(value3_Percentage, 0.0), 100.0);

  // Calculate pwm timings (pump with the highest value is set
  // to 100% pwm and the other two in relative to the max one)
  double maxValue_Percentage = max(1.0, max(pump1Clip_Percentage, max(pump2Clip_Percentage, pump3Clip_Percentage))); // 1.0->avoid divison by zero if all values are zero
  _pwmPump1_ms = (uint32_t)(pump1Clip_Percentage / maxValue_Percentage * _cycleTimespan_ms);
  _pwmPump2_ms = (uint32_t)(pump2Clip_Percentage / maxValue_Percentage * _cycleTimespan_ms);
  _pwmPump3_ms = (uint32_t)(pump3Clip_Percentage / maxValue_Percentage * _cycleTimespan_ms);

  ESP_LOGI(TAG, "Pump values changed to %d|%d|%d ms", _pwmPump1_ms, _pwmPump2_ms, _pwmPump3_ms);
}

//===============================================================
// Sets the cycle timespan in ms (200-1000ms)
//===============================================================
bool PumpDriver::SetCycleTimespan(uint32_t value_ms)
{
  // Check for min and max value
  if (value_ms < MIN_CYCLE_TIMESPAN_MS ||
    value_ms > MAX_CYCLE_TIMESPAN_MS)
  {
    return false;
  }

  // Set new value
  _cycleTimespan_ms = value_ms;
  ESP_LOGI(TAG, "Cycle timespan changed to %d ms", _cycleTimespan_ms);
  
  return true;
}

//===============================================================
// Returns the current cycle timespan
//===============================================================
uint32_t PumpDriver::GetCycleTimespan()
{
  return _cycleTimespan_ms;
}

//===============================================================
// Should be called every < 50 ms
//===============================================================
void PumpDriver::Update()
{
  // Save absolute time for pwm calculations
  uint32_t absoluteTime_ms = millis();

  // Add flow times from last update
  uint32_t flowTime_ms = absoluteTime_ms - _lastUpdate_ms;
  FlowMeter.AddFlowTime(_enablePump1 ? flowTime_ms : 0, _enablePump2 ? flowTime_ms : 0, _enablePump3 ? flowTime_ms : 0);

  // New cycle starts, reset last pump cycle timestamp
  if ((absoluteTime_ms - _lastPumpCycleStart_ms) > _cycleTimespan_ms)
  {
    _lastPumpCycleStart_ms = absoluteTime_ms;
  }

  // Calculate relative time within cycle
  uint32_t relativeTime_ms = absoluteTime_ms - _lastPumpCycleStart_ms;

  // Check if pumps must be powered on or off
  _enablePump1 = _isPumpEnabled && relativeTime_ms < _pwmPump1_ms;
  _enablePump2 = _isPumpEnabled && relativeTime_ms < _pwmPump2_ms;
  _enablePump3 = _isPumpEnabled && relativeTime_ms < _pwmPump3_ms;

  // Write digital pins
  digitalWrite(_pinPump1, _enablePump1 ? HIGH : LOW);
  digitalWrite(_pinPump2, _enablePump2 ? HIGH : LOW);
  digitalWrite(_pinPump3, _enablePump3 ? HIGH : LOW);

  // Avoid screensaver while dispensing
  if (_isPumpEnabled)
  {
    Systemhelper.SetLastUserAction();
  }

  // Check for pumps disable event
  if (_lastIsPumpEnabled &&
    !_isPumpEnabled)
  {
    // Save flow meter values to flash
    FlowMeter.Save();
  }

  // Save enabled state for next update
  _lastUpdate_ms = absoluteTime_ms;
  _lastIsPumpEnabled = _isPumpEnabled;
}

//===============================================================
// Enables or disables pump output
// IRAM_ATTR function: No communication !!
//===============================================================
void PumpDriver::Enable(bool enable)
{
  if (enable && Statemachine.CanEnablePumps())
  {
    // Enable pump power
    Pumps.InternalEnable();
  }
  else
  {
    // Disable pump power
    Pumps.InternalDisable();
  }
}

//===============================================================
// Enables pump output
// IRAM_ATTR function: No communication !!
//===============================================================
void PumpDriver::InternalEnable()
{
  // Set pins to output direction (enable)
  pinMode(_pinPump1, OUTPUT);
  pinMode(_pinPump2, OUTPUT);
  pinMode(_pinPump3, OUTPUT);
 
  // Set enabled flag to true
  // -> Update function is unlocked
  _isPumpEnabled = true;
}

//===============================================================
// Disables pump output
// IRAM_ATTR function: No communication !!
//===============================================================
void PumpDriver::InternalDisable()
{ 
  // Set enabled flag to false 
  // -> Update function is locked
  _isPumpEnabled = false;

  // Set pins to input direction (disable)
  pinMode(_pinPump1, INPUT);
  pinMode(_pinPump2, INPUT);
  pinMode(_pinPump3, INPUT);
  
  // Disable pumps, just to be sure
  digitalWrite(_pinPump1, LOW);
  digitalWrite(_pinPump2, LOW);
  digitalWrite(_pinPump3, LOW);
}
