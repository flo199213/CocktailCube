/*
 * CocktailCube main file
 *
 * @author    Florian Stäblein
 * @date      2025/01/01
 * @copyright © 2025 Florian Stäblein
 * 
 * ==============================================================
 * 
 * Configuration ESP32-S2:
 * - Board: "ESP32S2 Dev Module"
 * - CPU Frequency: "240MHz (WiFi)"
 * - USB CDC On Boot: "Enabled"   <------------ Important!
 * - Upload Mode: "Internal USB"  <------------ Important!
 * - Core Debug Level: "Verbose"  <------------ Only for Debugging
 * - USB DFU On Boot: "Disabled"
 * - USB Firmware MSC On Boot: "Disabled"
 * - Flash Size: "4Mb (32Mb)"
 * - Partition Scheme: "No OTA (2MB APP/2MB SPIFFS)"
 * - Upload Speed: "921600"
 * 
 * -> Leave everything else on default!
 * 
 * Important notice:
 * If the ESP32-S2 is programmed via the Arduino IDE, the
 * "USB CDC On Boot" flag must be set at all times. This flag
 * causes the ESP32-S2 to report as a COM interface immediately
 * after booting via USB. This means that the microcontroller can
 * be programmed via the Arduino Ide WITHOUT having to press the 
 * "BOOT" and "RESET" buttons again. (This allows the housing
 * to remain closed).
 * 
 * ==============================================================
 */

//===============================================================
// Includes
//===============================================================
#include <Arduino.h>
#include <SPI.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <Adafruit_ST7789.h>
#include <esp_log.h>
#include "Config.h"
#include "SystemHelper.h"
#include "StateMachine.h"
#include "EncoderButtonDriver.h"
#include "PumpDriver.h"
#include "DisplayDriver.h"
#include "FlowMeterDriver.h"
#include "WifiHandler.h"

//===============================================================
// Constants
//===============================================================
static const char* TAG = "main";

//===============================================================
// Defines
//===============================================================
// Rotary encoder pin defines
#define PIN_ENCODER_OUTA        8     // GPIO 8  -> rotary encoder output A
#define PIN_ENCODER_OUTB        11    // GPIO 11 -> rotary encoder output B
#define PIN_ENCODER_BUTTON      10    // GPIO 10 -> rotary encoder button

// Pumps pin defines
#define PIN_PUMP_1              1     // GPIO 1  -> pump 1 power
#define PIN_PUMP_2              2     // GPIO 2  -> pump 2 power
#define PIN_PUMP_3              4     // GPIO 4  -> pump 3 power
#define PIN_PUMPS_ENABLE        12    // GPIO 12 -> dispensing lever pumps enable, input
#define PIN_PUMPS_ENABLE_GND    13    // GPIO 13 -> dispensing lever pumps enable, GND

// Display pin defines
#define PIN_TFT_DC              37    // GPIO 37 -> TFT data/command
#define PIN_TFT_RST             38    // GPIO 38 -> TFT reset
#define PIN_TFT_CS              34    // GPIO 34 -> TFT chip select (Not connected! ST7789 is always selected)
#define PIN_TFT_SDA             35    // GPIO 35 -> TFT serial data input/output
#define PIN_TFT_SCL             36    // PGIO 36 -> TFT serial clock

// Buzzer pin defines
#define PIN_BUZZER              17    // GPIO 17 -> piezo buzzer

// LED pin defines
#define PIN_LEDLIGHT            6     // GPIO 6  -> power LEDs
#define PIN_LEDSTATUS           7     // GPIO 7  -> status LED

// Voltage measurement defines
#define PIN_VCC                 3     // GPIO 3  -> VCC voltage Custom PCB
#define VCC_CONVERSION_FACTOR   0.00766666666666666666666666666667 // 0-3.3V -> 0-25V

// Intro defines
#define INTRO_TIME_MS           3000  // Wait for 3 seconds at startup and show intro page

//===============================================================
// Global variables
//===============================================================
// Non-volatile preferences from flash
Preferences preferences;

// Display
Adafruit_ST7789* tft = NULL;

// Timer variables for alive counter
uint32_t aliveTimestampLoop = 0;
uint32_t aliveTimestampMain = 0;
const uint32_t AliveTime_ms = 2000;

// Timer variables for blink counter
uint32_t blinkTimestamp = 0;
const uint32_t BlinkTime_ms = 100;

//===============================================================
// Interrupt on pumps enable changing state
//===============================================================
void IRAM_ATTR ISR_Pumps_Enable()
{
  // If rising edge (button was released) -> disable pumps
  // If falling edge (button was pressed) -> enable pumps
  if (digitalRead(PIN_PUMPS_ENABLE))
  {
    // Disable pump power
    Pumps.Disable();

    // Request save flow values to flash
    FlowMeter.RequestSaveAsync();
  }
  else if (Statemachine.GetCurrentState() == eDashboard ||
    Statemachine.GetCurrentState() == eCleaning)
  {
    // Enable pump power
     Pumps.Enable();
  }

  // Update last user interaction
  Systemhelper.SetLastUserAction();
}

//===============================================================
// Interrupt on button changing state
//===============================================================
void IRAM_ATTR ISR_EncoderButton()
{
  EncoderButton.ButtonEvent();
}

//===============================================================
// Interrupt on encoder out A
//===============================================================
void IRAM_ATTR ISR_EncoderA()
{
  EncoderButton.DoEncoderA();
}

//===============================================================
// Interrupt on encoder out B
//===============================================================
void IRAM_ATTR ISR_EncoderB()
{
  EncoderButton.DoEncoderB();
}

//===============================================================
// Setup function
//===============================================================
void setup(void)
{
 // Start USB CDC // (redundant, for build verification purposes only)
  USBSerial.begin(); // <--- If you get an compile error here, you must
  // enable "USB CDC On Boot" in the Arduino IDE Target settings for ESP32-S2!

  // Initialize serial communication
  Serial.begin(115200);

  // Show startup log
  ESP_LOGI(TAG, "-- CocktailCube %s --", APP_VERSION);
  ESP_LOGI(TAG, "HeapSize : %d", ESP.getHeapSize());
  ESP_LOGI(TAG, "HeapFree : %d", ESP.getFreeHeap());

  // Initialize hardware SPI (Force HSPI)
  ESP_LOGI(TAG, "Initialize SPI");
  SPIClass *spi = new SPIClass(HSPI);
  spi->begin(PIN_TFT_SCL, -1, PIN_TFT_SDA, PIN_TFT_CS);

  // Initialize display
  ESP_LOGI(TAG, "Initialize display");
  tft = new Adafruit_ST7789(spi, PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);
  Display.Begin(tft);
  ESP_LOGI(TAG, "HeapSize : %d", ESP.getHeapSize());
  ESP_LOGI(TAG, "HeapFree : %d", ESP.getFreeHeap());

  // Initialize SPIFFS
  ESP_LOGI(TAG, "Initialize SPIFFS");
  SPIFFS.end(); // Close first for begin with 'formatOnFail'
  if (!SPIFFS.begin(true))
  {
    // Debug information on display
    Display.DrawInfoBox("Error", "Open SPIFFS failed");
    ESP_LOGE(TAG, "Loading SPIFFS images failed");
    delay(3000);
  }

  // Initialize system helper
  Systemhelper.Begin();

  // Loading config file
  ESP_LOGI(TAG, "Loading config file");
  if (!Config.Begin())
  {
    // Reset Config
    Config.ResetConfig();

    // Debug information on display
    Display.DrawInfoBox("Error", "Load config failed");
    ESP_LOGE(TAG, "Load config failed");
    delay(3000);
  }
  ESP_LOGI(TAG, "HeapSize : %d", ESP.getHeapSize());
  ESP_LOGI(TAG, "HeapFree : %d", ESP.getFreeHeap());
  
  // Loading images from SPIFFS
  ESP_LOGI(TAG, "Loading images from SPIFFS");
  Display.LoadImages();
  ESP_LOGI(TAG, "HeapSize : %d", ESP.getHeapSize());
  ESP_LOGI(TAG, "HeapFree : %d", ESP.getFreeHeap());
  
  // Show intro page
  ESP_LOGI(TAG, "Show intro page");
  Display.ShowIntroPage();
  uint32_t startupTime_ms = millis();

  // Initialize GPIOs
  ESP_LOGI(TAG, "Initialize GPIOs");
  pinMode(PIN_PUMPS_ENABLE, INPUT_PULLUP);
  pinMode(PIN_PUMPS_ENABLE_GND, OUTPUT);
  pinMode(PIN_LEDLIGHT, OUTPUT);
  pinMode(PIN_LEDSTATUS, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_VCC, INPUT);

  // Initialize outputs
  ESP_LOGI(TAG, "Initialize outputs");
  digitalWrite(PIN_PUMPS_ENABLE_GND, LOW); // Fixed GND value (0V, LOW)
  digitalWrite(PIN_LEDLIGHT, LOW);
  digitalWrite(PIN_LEDSTATUS, LOW);
  digitalWrite(PIN_BUZZER, LOW);

  // Play startup sound
	for (uint8_t index = 1; index <= 8; index++)
	{
    tone(PIN_BUZZER, index * 100, 65);
    delay(85);
  }

  // Initialize encoder button
  ESP_LOGI(TAG, "Initialize encoder button");
  EncoderButton.Begin(PIN_ENCODER_OUTA, PIN_ENCODER_OUTB, PIN_ENCODER_BUTTON);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_OUTA), ISR_EncoderA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_OUTB), ISR_EncoderB, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_BUTTON), ISR_EncoderButton, CHANGE);

  // Initialize flow values from EEPROM
  ESP_LOGI(TAG, "Initialize flow values from EEPROM");
  FlowMeter.Load();

  // Get VCC voltage (Only for Custom PCB)
  double vccVoltage = analogReadMilliVolts(PIN_VCC) * VCC_CONVERSION_FACTOR;
  ESP_LOGI(TAG, "Get VCC voltage: %0.2f", vccVoltage);
  
  // Initialize pump driver
  ESP_LOGI(TAG, "Initialize pump driver");
  Pumps.Begin(PIN_PUMP_1, PIN_PUMP_2, PIN_PUMP_3, vccVoltage);

  // Initialize state machine
  ESP_LOGI(TAG, "Initialize state machine");
  Statemachine.Begin(PIN_BUZZER);
  
  // Wait for the rest of the intro time
  while ((millis() - startupTime_ms) < INTRO_TIME_MS) { };

  // Allow interrupts for encoder button
  sei();

  // Show help page until button is pressed
  ESP_LOGI(TAG, "Show help page");
  Display.ShowHelpPage();
  bool infoBoxShown = false;
  while (true)
  {
    // If someone turns the knob, not knowing what to do, display the help text
    if (!infoBoxShown && EncoderButton.GetEncoderIncrements() > 0)
    {
      // Draw info box with help text
      Display.DrawInfoBox("Press Button", "to start!");
      infoBoxShown = true;
    }

    // Check for button press user input
    if (EncoderButton.IsButtonPress())
    {
      // Play short beep sound and proceed with setup
      tone(PIN_BUZZER, 500, 40);
      break;
    }

    // Contains yield() for ESP32
    delay(1);
  }

  // Initialize wifi
  ESP_LOGI(TAG, "Initialize wifi");
  Wifihandler.Begin();

  // Initial run of state machine with entry event
  ESP_LOGI(TAG, "Initial run of state machine");
  Statemachine.Execute(eEntry);

  // Initialize interrupt for dispenser lever
  ESP_LOGI(TAG, "Initialize interrupt for dispenser lever");
  attachInterrupt(digitalPinToInterrupt(PIN_PUMPS_ENABLE), ISR_Pumps_Enable, CHANGE);

  // Final output
  ESP_LOGI(TAG, "Setup Finished");
}

//===============================================================
// Loop function
//===============================================================
void loop()
{
  // Show debug alive message
  if ((millis() - aliveTimestampLoop) > AliveTime_ms)
  {
    aliveTimestampLoop = millis();
    ESP_LOGI(TAG, "Loop Alive");
    
    // Print mixture information
    ESP_LOGI(TAG, "%s", Statemachine.GetMixtureString().c_str());

    // Print memory information
    ESP_LOGI(TAG, "%s", Systemhelper.GetMemoryInfoString().c_str());

    // Toggle status LED
    digitalWrite(PIN_LEDSTATUS, !digitalRead(PIN_LEDSTATUS));
  }

  // Flash LED light if dispensing is in progress
  if (Pumps.IsEnabled())
  {
    // Set LED to blink
    if ((millis() - blinkTimestamp) > BlinkTime_ms)
    {
      blinkTimestamp = millis();
      digitalWrite(PIN_LEDLIGHT, !digitalRead(PIN_LEDLIGHT));
    }
  }
  else
  {
    // Set LED to on
    digitalWrite(PIN_LEDLIGHT, HIGH);
  }

  // Update pump outputs
  Pumps.Update();

  // Save flow meter values to flash if requested
  FlowMeter.SaveAsync();

  // Run statemachine with main task event
  Statemachine.Execute(eMain);

  // Update wifi, webserver and clients
  Wifihandler.Update();
}
