/**
 * CocktailCube hardware check V1.0
 *
 * @author    Florian Staeblein
 * @date      2023/09/15
 * @copyright © 2023 Florian Staeblein
 * 
 * ==============================================================
 * 
 * Configuration ESP32-S2:
 * - Board: "ESP32S2 Dev Module"
 * - CPU Frequency: "240MHz (WiFi)"
 * - USB CDC On Boot: "Enabled"   <------------ Important!
 * - USB DFU On Boot: "Disabled"
 * - USB Firmware MSC On Boot: "Disabled"
 * - Flash Size: "4Mb (32Mb)"
 * - Partition Scheme: "No OTA (2MB APP/2MB SPIFFS)"
 * - Upload Mode: "Internal USB"
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
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

//===============================================================
// Defines
//===============================================================
// Rotary encoder pin defines
#define PIN_ENCODER_OUTA        8   // GPIO 8  -> rotary encoder output A
#define PIN_ENCODER_OUTB        11  // GPIO 11 -> rotary encoder output B
#define PIN_ENCODER_BUTTON      10  // GPIO 10 -> rotary encoder button

// Pump pin defines
#define PIN_PUMP_1              1   // GPIO 1  -> pump 1 power
#define PIN_PUMP_2              2   // GPIO 2  -> pump 2 power
#define PIN_PUMP_3              4   // GPIO 4  -> pump 3 power
#define PIN_PUMPS_ENABLE        12  // GPIO 12 -> dispensing lever pump enable, input
#define PIN_PUMPS_ENABLE_GND    13  // GPIO 13 -> dispensing lever pump enable, GND

// Display pin defines
#define PIN_TFT_DC              37  // GPIO 37 -> TFT data/command
#define PIN_TFT_RST             38  // GPIO 38 -> TFT reset
#define PIN_TFT_CS              34  // GPIO 34 -> TFT chip select (Not connected! ST7789 is always selected)
#define PIN_TFT_SDA             35  // GPIO 35 -> TFT serial data input/output
#define PIN_TFT_SCL             36  // PGIO 36 -> TFT serial clock

// Buzzer pin defines
#define PIN_BUZZER              17  // GPIO 17 -> piezo buzzer

// LED pin defines
#define PIN_LEDLIGHT            6   // GPIO 6  -> power LEDs
#define PIN_LEDSTATUS           7   // GPIO 7  -> status LED

// TFT defines
#define TFT_WIDTH               240
#define TFT_HEIGHT              240
#define TFT_COLOR_BACKGROUND    ST77XX_BLACK
#define TFT_COLOR_TEXT          ST77XX_ORANGE
#define TFT_COLOR_ALIVE         ST77XX_RED

//===============================================================
// Global variables
//===============================================================
Adafruit_ST7789 *tft = NULL;

// Timer variables for alive counter
uint32_t aliveTimestamp = 0;
const uint32_t AliveTime_ms = 5000;

bool isAliveActive = false;
bool isLeverActive = false;
bool isButtonActive = false;
bool isEncoderAActive = false;
bool isEncoderBActive = false;

//===============================================================
// Setup function
//===============================================================
void setup(void)
{
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("[SETUP] Check V1.0");

  // Init SPI
  SPIClass *spi = new SPIClass(HSPI);
  spi->begin(PIN_TFT_SCL, -1, PIN_TFT_SDA, PIN_TFT_CS);

  // Init display
  tft = new Adafruit_ST7789(spi, PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);
  tft->init(TFT_WIDTH, TFT_HEIGHT, SPI_MODE3);
  tft->invertDisplay(true);
  tft->setRotation(3);
  tft->setTextWrap(false);
  tft->fillScreen(TFT_COLOR_BACKGROUND);

  // Initialize GPIOs
  pinMode(PIN_ENCODER_OUTA, INPUT_PULLUP);
  pinMode(PIN_ENCODER_OUTB, INPUT_PULLUP);
  pinMode(PIN_ENCODER_BUTTON, INPUT_PULLUP); 
  pinMode(PIN_PUMP_1, OUTPUT);
  pinMode(PIN_PUMP_2, OUTPUT);
  pinMode(PIN_PUMP_3, OUTPUT);
  pinMode(PIN_PUMPS_ENABLE, INPUT_PULLUP);
  pinMode(PIN_PUMPS_ENABLE_GND, OUTPUT);
  pinMode(PIN_TFT_DC, OUTPUT);
  pinMode(PIN_TFT_RST, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LEDLIGHT, OUTPUT);

  // Initialize outputs
  digitalWrite(PIN_PUMP_1, LOW);
  digitalWrite(PIN_PUMP_2, LOW);
  digitalWrite(PIN_PUMP_3, LOW);
  digitalWrite(PIN_PUMPS_ENABLE_GND, LOW);
  digitalWrite(PIN_BUZZER, LOW);
  digitalWrite(PIN_LEDLIGHT, LOW);

  // Start check
  // Check pump 1
  digitalWrite(PIN_PUMP_1, HIGH);
  delay(500);
  digitalWrite(PIN_PUMP_1, LOW);
  
  // Check pump 2
  digitalWrite(PIN_PUMP_2, HIGH);
  delay(500);
  digitalWrite(PIN_PUMP_2, LOW);
  
  // Check pump 3
  digitalWrite(PIN_PUMP_3, HIGH);
  delay(500);
  digitalWrite(PIN_PUMP_3, LOW);
  
  // Check LEDs
  digitalWrite(PIN_LEDLIGHT, HIGH);
  delay(500);
  digitalWrite(PIN_LEDLIGHT, LOW);

  // Check buzzer
  tone(PIN_BUZZER, 800, 500);

  // Check display
  tft->fillScreen(TFT_COLOR_BACKGROUND);
  DrawCenteredString("Check 'OK'", tft->width() / 2, tft->height() / 2, TFT_COLOR_TEXT);

  Serial.println("[SETUP] Finished");
}

//===============================================================
// Loop function
//===============================================================
void loop()
{
  // Show debug alive message
  if ((millis() - aliveTimestamp) > AliveTime_ms)
  {
    aliveTimestamp = millis();
    Serial.println("[LOOP] Alive");

    if (isAliveActive)
    {
      DrawCenteredString("Alive!", tft->width() / 2, 20, TFT_COLOR_ALIVE);
    }
    else
    {
      DrawCenteredString("Alive!", tft->width() / 2, 20, TFT_COLOR_BACKGROUND);
    }

    isAliveActive = !isAliveActive;
  }

  // Check lever
  if (!isLeverActive && 
    digitalRead(PIN_PUMPS_ENABLE) == LOW)
  {
    isLeverActive = true;
    
    // Short beep sound
    tone(PIN_BUZZER, 500, 40);

    // Show information
    tft->fillRect(0, 30, tft->width(), tft->height() - 30, TFT_COLOR_BACKGROUND);
    DrawCenteredString("Check 'Lever'", tft->width() / 2, tft->height() / 2, TFT_COLOR_TEXT);

    // Debouncing
    delay(100);
  }
  else
  {
    isLeverActive = false;
  }

  // Check encoder button
  if (!isButtonActive && 
    digitalRead(PIN_ENCODER_BUTTON) == LOW)
  {
    isButtonActive = true;
    
    // Short beep sound
    tone(PIN_BUZZER, 500, 40);

    // Show information
    tft->fillRect(0, 30, tft->width(), tft->height() - 30, TFT_COLOR_BACKGROUND);
    DrawCenteredString("Check 'Button'", tft->width() / 2, tft->height() / 2, TFT_COLOR_TEXT);

    // Debouncing
    delay(100);
  }
  else
  {
    isButtonActive = false;
  }
  
  // Check encoder output A
  if (!isEncoderAActive && 
    digitalRead(PIN_ENCODER_OUTA) == LOW)
  {
    isEncoderAActive = true;
    
    // Short beep sound
    tone(PIN_BUZZER, 500, 40);

    // Show information
    tft->fillRect(0, 30, tft->width(), tft->height() - 30, TFT_COLOR_BACKGROUND);
    DrawCenteredString("Check 'Enc A'", tft->width() / 2, tft->height() / 2, TFT_COLOR_TEXT);

    // Debouncing
    delay(100);
  }
  else
  {
    isEncoderAActive = false;
  }
  
  // Check encoder output B
  if (!isEncoderBActive && 
    digitalRead(PIN_ENCODER_OUTB) == LOW)
  {
    isEncoderBActive = true;
    
    // Short beep sound
    tone(PIN_BUZZER, 500, 40);

    // Show information
    tft->fillRect(0, 30, tft->width(), tft->height() - 30, TFT_COLOR_BACKGROUND);
    DrawCenteredString("Check 'Enc B'", tft->width() / 2, tft->height() / 2, TFT_COLOR_TEXT);

    // Debouncing
    delay(100);
  }
  else
  {
    isEncoderBActive = false;
  }
}

//===============================================================
// Draws a string centered
//===============================================================
void DrawCenteredString(const String &text, int16_t x, int16_t y, uint16_t color)
{
  tft->setTextSize(2);
  tft->setTextColor(color);

  int16_t x1, y1;
  uint16_t w, h;
  tft->getTextBounds(text, x, y, &x1, &y1, &w, &h);
  
  int16_t x_text = x - w / 2;
  int16_t y_text = y - h / 2;
  tft->setCursor(x_text, y_text);
  tft->print(text);
}
