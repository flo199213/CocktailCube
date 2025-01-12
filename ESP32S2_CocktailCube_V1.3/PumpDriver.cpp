/**
 * Includes pump driver functions
 *
 * @author    Florian Staeblein
 * @date      2024/01/28
 * @copyright © 2024 Florian Staeblein
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
  Pumps.Load();

  // Disable pump output
  DisableInternal();
  
  // Log startup info
  ESP_LOGI(TAG, "Finished initializing pump driver");
}

//===============================================================
// Load settings from flash
//===============================================================
void PumpDriver::Load()
{
  if (_preferences.begin(SETTINGS_NAME, true))
  {
    _preferences.begin(SETTINGS_NAME, false);
    _cycleTimespan_ms = _preferences.getLong(KEY_CYCLETIMESPAN_MS, DEFAULT_CYCLE_TIMESPAN_MS);
    _preferences.end();

    ESP_LOGI(TAG, "Preferences successfully loaded from '%s'", SETTINGS_NAME);
  }
  else
  {
    ESP_LOGE(TAG, "Could not open preferences '%s'", SETTINGS_NAME);
  }
}

//===============================================================
// Save settings to flash
//===============================================================
void PumpDriver::Save()
{
  if (_preferences.begin(SETTINGS_NAME, false))
  {
    _preferences.begin(SETTINGS_NAME, false);
    _preferences.putLong(KEY_CYCLETIMESPAN_MS, _cycleTimespan_ms);
    _preferences.end();

    ESP_LOGI(TAG, "Preferences successfully saved to '%s'", SETTINGS_NAME);
  }
  else
  {
    ESP_LOGE(TAG, "Could not open preferences '%s'", SETTINGS_NAME);
  }
}

//===============================================================
// Enables pump output
//===============================================================
void PumpDriver::Enable()
{
  if (_isPumpEnabled)
  {
    return;
  }

  // Enable internal gpios
  EnableInternal();
 
  // Set enabled flag to true
  // -> Update function is unlocked
  _isPumpEnabled = true;

  // Set log info
  ESP_LOGI(TAG, "Pumps enabled");
}

//===============================================================
// Disables pump output
//===============================================================
void PumpDriver::Disable()
{
  if (!_isPumpEnabled)
  {
    return;
  }
  
  // Set enabled flag to false 
  // -> Update function is locked
  _isPumpEnabled = false;

  // Disable internal gpios
  DisableInternal();
  
  // Set log info
  ESP_LOGI(TAG, "Pumps disabled");
}

//===============================================================
// Enables pump output (internal)
//===============================================================
void PumpDriver::EnableInternal()
{
  // Set pins to output direction (enable)
  pinMode(_pinPump1, OUTPUT);
  pinMode(_pinPump2, OUTPUT);
  pinMode(_pinPump3, OUTPUT);
}

//===============================================================
// Disables pump output (internal)
//===============================================================
void PumpDriver::DisableInternal()
{
  // Set pins to input direction (disable)
  pinMode(_pinPump1, INPUT);
  pinMode(_pinPump2, INPUT);
  pinMode(_pinPump3, INPUT);
  
  // Disable pumps, just to be sure
  digitalWrite(_pinPump1, LOW);
  digitalWrite(_pinPump2, LOW);
  digitalWrite(_pinPump3, LOW);

  // Save timestamp
  uint32_t onTimestamp_ms = _lastPumpCycleStart_ms;
  uint32_t offTimestamp_ms = millis();

  // Add already passed flow time to flow meter if pump is not already off
  uint32_t passedFlowTime = offTimestamp_ms - onTimestamp_ms;
  uint32_t flowTime1_ms = _lastEnablePump1 == true ? passedFlowTime : 0;
  uint32_t flowTime2_ms = _lastEnablePump2 == true ? passedFlowTime : 0;
  uint32_t flowTime3_ms = _lastEnablePump3 == true ? passedFlowTime : 0;
  FlowMeter.AddFlowTime(flowTime1_ms, flowTime2_ms, flowTime3_ms);

  // All pumps are now disabled
  _lastEnablePump1 = false;
  _lastEnablePump2 = false;
  _lastEnablePump3 = false;
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
  if (!_isPumpEnabled)
  {
    return;
  }
  
  // Save absolute time for pwm calculations
  uint32_t absoluteTime_ms = millis();

  // New cycle starts, reset last pump cycle timestamp
  if ((absoluteTime_ms - _lastPumpCycleStart_ms) > _cycleTimespan_ms)
  {
    _lastPumpCycleStart_ms = absoluteTime_ms;
  }

  // Calculate relative time within cycle
  uint32_t relativeTime_ms = absoluteTime_ms - _lastPumpCycleStart_ms;

  // Check if pumps must be powered on or off
  bool enablePump1 = relativeTime_ms < _pwmPump1_ms;
  bool enablePump2 = relativeTime_ms < _pwmPump2_ms;
  bool enablePump3 = relativeTime_ms < _pwmPump3_ms;

  // Write digital pins
  digitalWrite(_pinPump1, enablePump1 ? HIGH : LOW);
  digitalWrite(_pinPump2, enablePump2 ? HIGH : LOW);
  digitalWrite(_pinPump3, enablePump3 ? HIGH : LOW);

  // Add flow times when powering off (falling edge)
  uint32_t flowTime1_ms = _lastEnablePump1 == true && enablePump1 == false ? _pwmPump1_ms : 0;
  uint32_t flowTime2_ms = _lastEnablePump2 == true && enablePump2 == false ? _pwmPump2_ms : 0;
  uint32_t flowTime3_ms = _lastEnablePump3 == true && enablePump3 == false ? _pwmPump3_ms : 0;
  FlowMeter.AddFlowTime(flowTime1_ms, flowTime2_ms, flowTime3_ms);

  // Save enabled state for next update
  _lastEnablePump1 = enablePump1;
  _lastEnablePump2 = enablePump2;
  _lastEnablePump3 = enablePump3;
}
