#include "Arduino.h"
/**
 * Includes a spiffs image
 *
 * @author    Florian Stäblein
 * @date      2025/01/01
 * @copyright © 2025 Florian Stäblein
 */

//===============================================================
// Includes
//===============================================================
#include "SPIFFSImage.h"

//===============================================================
// Constants
//===============================================================
static const char* TAG = "spiffsimage";

//===============================================================
// Constructor
//===============================================================
SPIFFSImage::SPIFFSImage()
{
}

//===============================================================
// Destructor
//===============================================================
SPIFFSImage::~SPIFFSImage()
{
  Deallocate();
}

//===============================================================
// Allocates the internal buffer
//===============================================================
bool SPIFFSImage::Allocate(int16_t width, int16_t height)
{
  ESP_LOGI(TAG, "Allocating new SPIFFS image");

  // Get maximal bytes which are allowed to alloc
  uint32_t maxAllocBytes = ESP.getMaxAllocHeap();

  // Check before alloc for enough heap (RAM)
  uint32_t requiredBytes = width * height * 2; // width * height * 2 bytes (uint16_t color)
  if (requiredBytes >= maxAllocBytes)
  {
    return false;
  }

  // Set size
  _width = width;
  _height = height;

  // Allocate buffer
  if ((_buffer = (uint16_t*)malloc(requiredBytes)))
  {
    memset(_buffer, 0, requiredBytes);
    ESP_LOGI(TAG, "New SPIFFS image allocated");

    return true;
  }

  return false;
}

//===============================================================
// Deallocates the internal buffer
//===============================================================
void SPIFFSImage::Deallocate()
{  
  // Set size
  _height = 0;
  _width = 0;

  // Deallocate buffer
  if (_buffer)
  {
    free(_buffer);
    ESP_LOGI(TAG, "SPIFFS image buffer is free");
  }
}

//===============================================================
// Draws the canvas on the tft
//===============================================================
void SPIFFSImage::Draw(int16_t x, int16_t y, Adafruit_SPITFT* tft, uint16_t transparencyColor, uint16_t shadowColor, bool asShadow)
{
  if (!_buffer)
  {
    return;
  }

  // Write pixels
  tft->startWrite();
  for (int16_t row = 0; row < _height; row++)
  {
    for (int16_t column = 0; column < _width; column++)
    {
      uint16_t currentColor = _buffer[row * _width + column];
      if (currentColor != transparencyColor)
      {
        tft->writePixel(x + column, y + row, asShadow ? shadowColor : currentColor);
      }
    }
  }
  tft->endWrite();
}

//===============================================================
// Clears the difference between two images
//===============================================================
void SPIFFSImage::ClearDiff(int16_t x0, int16_t y0, int16_t x1, int16_t y1, SPIFFSImage* otherImage, Adafruit_SPITFT* tft, uint16_t transparencyColor, uint16_t clearColor)
{
  if (otherImage == NULL)
  {
    return;
  }
 
  uint16_t* otherBuffer = otherImage->GetBuffer();
  int16_t otherHeight = otherImage->Height();
  int16_t otherWidth = otherImage->Width();

  // Write pixels
  tft->startWrite();
  for (int16_t row = 0; row < _height; row++)
  {
    for (int16_t column = 0; column < _width; column++)
    {
      uint16_t currentColor = _buffer[row * _width + column];
      
      // Calculate other indexes
      int16_t otherColumn = column - (x1 - x0);
      int16_t otherRow = row - (y1 - y0);

      uint16_t otherColor = transparencyColor + 1; // Use color != transparencyColor
      if (otherColumn > 0 && otherColumn < otherWidth &&
        otherRow > 0 && otherRow < otherHeight)
      {
        otherColor = otherBuffer[otherRow * otherWidth + otherColumn];
      }

      // Clear color, if current color is not transparent and other color is (must be reset)
      if (currentColor != transparencyColor &&
        otherColor == transparencyColor)
      {
        tft->writePixel(x0 + column, y0 + row, clearColor);
      }
    }
  }
  tft->endWrite();
}

//===============================================================
// Moves the canvas on the tft
//===============================================================
void SPIFFSImage::Move(int16_t x0, int16_t y0, int16_t x1, int16_t y1, Adafruit_SPITFT* tft, uint16_t clearColor, uint16_t transparencyColor, bool onlyClear)
{
  if (!_buffer)
  {
    return;
  }

  // Clear old image (only diff to new one, to avoid flickering)
  tft->startWrite();
  for (int16_t row = 0; row < _height; row++)
  {
    for (int16_t column = 0; column < _width; column++)
    {
      // Get old color value
      uint16_t colorOld = _buffer[row * _width + column];

      // Calculate new color indexes
      int16_t newColumn = column - (x1 - x0);
      int16_t newRow = row - (y1 - y0);
      
      // Get new color value
      uint16_t colorNew = transparencyColor;
      if (newColumn > 0 && newColumn < _width &&
        newRow > 0 && newRow < _height)
      {
        colorNew = _buffer[newRow * _width + newColumn];
      }
      
      // Reset pixel only if the color would be transparent and old color was not
      if (colorOld != transparencyColor &&
        colorNew == transparencyColor)
      {
        tft->writePixel(x0 + column, y0 + row, clearColor);
      }
    }
  }
  tft->endWrite();
  
  if (!onlyClear)
  {
    // Draw new (moved) image
    Draw(x1, y1, tft, transparencyColor);
  }
}

//===============================================================
// Return a pixel at the requested position
//===============================================================
uint16_t SPIFFSImage::GetPixel(int16_t x, int16_t y)
{
  int16_t index = y * _width + x;
  if (_buffer &&
    index < _width * _height)
  {
    return _buffer[index];
  }
  
  return 0;
}
