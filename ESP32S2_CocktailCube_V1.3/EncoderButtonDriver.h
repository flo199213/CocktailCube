/*
 * Includes all encoder and button functions
 *
 * @author    Florian Stäblein
 * @date      2025/01/01
 * @copyright © 2025 Florian Stäblein
 */
 
#ifndef ENCODERBUTTONDRIVER_H
#define ENCODERBUTTONDRIVER_H

//===============================================================
// Includes
//===============================================================
#include <Arduino.h>
#include <esp_log.h>
#include "Config.h"
#include "SystemHelper.h"

//===============================================================
// Defines
//===============================================================
#define MINIMUMLONGTIMEPRESS_MS   500

//===============================================================
// Class for handling encoder and button functions
//===============================================================
class EncoderButtonDriver
{
  public:
    // Constructor
    EncoderButtonDriver();

    // Initializes the encoder and button driver
    void Begin(uint8_t pinEncoderOutA, uint8_t pinEncoderOutB, uint8_t pinEncoderButton);
    
    // Return true, if a button press is pending. Otherwise false
    bool IsButtonPress();
    
    // Return true, if a long button press is pending. Otherwise false
    bool IsLongButtonPress();

    // Returns the counted encoder pulses since the last query and resets the counter
    int16_t GetEncoderIncrements();
    
    // Should be called if button was pressed or released
    void IRAM_ATTR ButtonEvent();

    // Should be called if encoder edge A changed
    void IRAM_ATTR DoEncoderA();

    // Should be called if encoder edge B changed
    void IRAM_ATTR DoEncoderB();

  private:
    // Port mux
    portMUX_TYPE _mux = portMUX_INITIALIZER_UNLOCKED;

    // Pin definitions
    volatile uint8_t _pinEncoderOutA;                 // volatile for ISR use
    volatile uint8_t _pinEncoderOutB;                 // volatile for ISR use
    volatile uint8_t _pinEncoderButton;               // volatile for ISR use

    // Interrupt service routine variables
    volatile bool _A_set = false;                     // volatile for ISR use
    volatile bool _B_set = false;                     // volatile for ISR use

    // Rotary encoder variables
    volatile int16_t _encoderIncrements = 0;          // volatile for ISR use

    // Encoder state variables
    volatile bool _isButtonPress = false;             // volatile for ISR use
    volatile uint32_t _lastButtonPress_ms = 0;        // volatile for ISR use
    volatile bool _suppressShortButtonPress = false;  // volatile for ISR use
};

//===============================================================
// Global variables
//===============================================================
extern EncoderButtonDriver EncoderButton;

#endif
