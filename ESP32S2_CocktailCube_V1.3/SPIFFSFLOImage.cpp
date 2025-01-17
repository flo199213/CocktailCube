#include "Arduino.h"
/**
 * Includes a spiffs FLO image
 *
 * @author    Florian Stäblein
 * @date      2025/01/01
 * @copyright © 2025 Florian Stäblein
 */

//===============================================================
// Includes
//===============================================================
#include "SPIFFSFLOImage.h"

//===============================================================
// Constants
//===============================================================
static const char* TAG = "floimage";

//===============================================================
// Constructor
//===============================================================
SPIFFSFLOImage::SPIFFSFLOImage()
{
}

//===============================================================
// Destructor
//===============================================================
SPIFFSFLOImage::~SPIFFSFLOImage()
{
  Deallocate();
}

//===============================================================
// Allocates the internal buffer
//===============================================================
ImageReturnCode SPIFFSFLOImage::Allocate(String filename)
{
  ESP_LOGI(TAG, "Allocating new SPIFFS image '%s'", filename.c_str());

  /*
  FLO content overview:
  --------------------------------------
  Header
    Width                (16 bit value)
    Height               (16 bit value)
  --------------------------------------
  Color Table (16 colors)
    Color 1              (16 bit color, transparency color by default)
    Color 2              (16 bit color)
    ...
    Color 15             (16 bit color)
  --------------------------------------
  Pixel Data
    Pixel 1              (8 bit color index)
    Pixel 2              (8 bit color index)
    Block n              (8 bit color index)
    ...
  --------------------------------------
  */

  // Correct path if it does not start with "/"
  filename.trim();
  if (!filename.startsWith("/"))
  {
    filename = "/" + filename;
  }

  // Open requested file on SPIFFS
  if (!(_file = SPIFFS.open(filename, FILE_READ)))
  {
    return IMAGE_ERR_FILE_NOT_FOUND;
  }

  // Parse FLO header and check signature
  char signature[3];
  _file.readBytes(signature, 3);
  if (strncmp(signature, "FLO", 3) != 0)
  {
    _file.close();
    return IMAGE_ERR_FORMAT;
  }

  // Read image properties
  _header.width = ReadLE16(&_file);
  _header.height = ReadLE16(&_file);
  
  // Read color table
  size_t tableByteSize = MAX_TABLE_SIZE * 2; // * 2 -> colors are uint16_t
  if (_file.read((uint8_t*)_header.colorTable, tableByteSize) != tableByteSize)
  {
    _file.close();
    return IMAGE_ERR_TABLE;
  }

  // Get maximal bytes which are allowed to alloc
  uint32_t maxAllocBytes = ESP.getMaxAllocHeap();

  // Calculate pixel data byte size (4bit * width * height)
  size_t pixelDataByteSize = ceil(_header.width * _header.height / 2.0f);

  // Check for enough heap (RAM) and allocate buffer
  if (pixelDataByteSize >= maxAllocBytes ||
    !(_buffer = (uint8_t*)malloc(pixelDataByteSize)))
  {
    _file.close();
    return IMAGE_ERR_MALLOC;
  }

  // Set buffer to zero
  memset(_buffer, 0, pixelDataByteSize);
  ESP_LOGI(TAG, "New FLO buffer allocated (Heap: %d / %d Bytes)", ESP.getFreeHeap(), ESP.getHeapSize());

  // Read pixel data
  size_t pixelDataByteSizeRead = _file.read(_buffer, pixelDataByteSize);
  if (pixelDataByteSizeRead != pixelDataByteSize)
  {
    _file.close();
    return IMAGE_ERR_PIXELDATA;
  }

  // Set valid fag
  _isValid = true;
  
  _file.close();
  return IMAGE_SUCCESS;
}

//===============================================================
// Deallocates the internal buffer
//===============================================================
void SPIFFSFLOImage::Deallocate()
{
  // Remove valid fag
  _isValid = false;

  // Set size
  _header.height = 0;
  _header.width = 0;
  
  if (_file)
  {
    _file.close();
  }

  // Deallocate buffer
  if (_buffer)
  {
    free(_buffer);
    ESP_LOGI(TAG, "FLO image buffer is free");
  }
}

//===============================================================
// Return a pixel at the requested position
//===============================================================
uint16_t SPIFFSFLOImage::GetPixel(int16_t x, int16_t y)
{
  int16_t index = (y * _header.width + x) / 2;
  bool isHighNibble = (x % 2) == 0;

  if (_buffer &&
    index >= 0 &&
    index < (_header.width * _header.height + 1) / 2)
  {
    uint8_t byteValue = _buffer[index];

    uint8_t pixelIndex = isHighNibble
      ? (byteValue >> 4) & 0x0F // High Nibble
      : byteValue & 0x0F;       // Low Nibble

    return _header.colorTable[pixelIndex];
  }

  return 0;
}

//===============================================================
// Draws the canvas on the tft
//===============================================================
void SPIFFSFLOImage::Draw(int16_t x, int16_t y, Adafruit_SPITFT* tft, uint16_t shadowColor, bool asShadow)
{
  if (!_buffer)
  {
    return;
  }

  // Write pixels
  tft->startWrite();
  for (int16_t row = 0; row < _header.height; row++)
  {
    for (int16_t column = 0; column < _header.width; column++)
    {
      uint16_t currentColor = GetPixel(column, row);
      if (currentColor != _header.colorTable[0])
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
void SPIFFSFLOImage::ClearDiff(int16_t x0, int16_t y0, int16_t x1, int16_t y1, SPIFFSFLOImage* otherImage, Adafruit_SPITFT* tft, uint16_t clearColor)
{
  if (otherImage == NULL)
  {
    return;
  }
 
  int16_t otherHeight = otherImage->Height();
  int16_t otherWidth = otherImage->Width();

  // Write pixels
  tft->startWrite();
  for (int16_t row = 0; row < _header.height; row++)
  {
    for (int16_t column = 0; column < _header.width; column++)
    {
      uint16_t currentColor = GetPixel(column, row);
      
      // Calculate other indexes
      int16_t otherColumn = column - (x1 - x0);
      int16_t otherRow = row - (y1 - y0);

      uint16_t otherColor = _header.colorTable[0] + 1; // Use color != transparencyColor
      if (otherColumn > 0 && otherColumn < otherWidth &&
        otherRow > 0 && otherRow < otherHeight)
      {
        otherColor = otherImage->GetPixel(otherColumn, otherRow);
      }

      // Clear color, if current color is not transparent and other color is transparent (must be reset)
      if (currentColor != _header.colorTable[0] &&
        otherColor == _header.colorTable[0])
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
void SPIFFSFLOImage::Move(int16_t x0, int16_t y0, int16_t x1, int16_t y1, Adafruit_SPITFT* tft, uint16_t clearColor, bool onlyClear)
{
  if (!_buffer)
  {
    return;
  }

  // Clear old image (only diff to new one, to avoid flickering)
  tft->startWrite();
  for (int16_t row = 0; row < _header.height; row++)
  {
    for (int16_t column = 0; column < _header.width; column++)
    {
      // Get old color value
      uint16_t colorOld = GetPixel(column, row);

      // Calculate new color indexes
      int16_t newColumn = column - (x1 - x0);
      int16_t newRow = row - (y1 - y0);
      
      // Get new color value
      uint16_t colorNew = _header.colorTable[0];
      if (newColumn > 0 && newColumn < _header.width &&
        newRow > 0 && newRow < _header.height)
      {
        colorNew = GetPixel(newColumn, newRow);
      }
      
      // Reset pixel only if the color would be transparent and old color was not
      if (colorOld != _header.colorTable[0] &&
        colorNew == _header.colorTable[0])
      {
        tft->writePixel(x0 + column, y0 + row, clearColor);
      }
    }
  }
  tft->endWrite();
  
  if (!onlyClear)
  {
    // Draw new (moved) image
    Draw(x1, y1, tft, _header.colorTable[0]);
  }
}

//===============================================================
// Print error code string to stream
//===============================================================
String SPIFFSFLOImage::PrintStatus(ImageReturnCode stat)
{
  if (stat == IMAGE_SUCCESS)
  {
    return String("Success!");
  }
  else if (stat == IMAGE_ERR_FILE_NOT_FOUND)
  {
    return String("File not found.");
  }
  else if (stat == IMAGE_ERR_FORMAT)
  {
    return String("Not a supported FLO variant.");
  }
  else if (stat == IMAGE_ERR_HEADER)
  {
    return String("Does not contain FLO header.");
  }
  else if (stat == IMAGE_ERR_TABLE)
  {
    return String("Does not contain color table.");
  }
  else if (stat == IMAGE_ERR_MALLOC)
  {
    return String("Malloc failed (insufficient RAM).");
  }
  else if (stat == IMAGE_ERR_PIXELDATA)
  {
    return String("Not enough pixel data read.");
  }

  return "Unknown";
}

//===============================================================
// Reads a little-endian 16-bit unsigned value from currently-
// open File, converting if necessary to the microcontroller's
// native endianism. (BMP files use little-endian values.)
//===============================================================
uint16_t SPIFFSFLOImage::ReadLE16(File* file)
{
  // Big-endian or unknown. Byte-by-byte read will perform reversal if needed.
  return file->read() | ((uint16_t)file->read() << 8);
}

//===============================================================
// Reads a little-endian 32-bit unsigned value from currently-
// open File, converting if necessary to the microcontroller's
// native endianism. (BMP files use little-endian values.)
//===============================================================
uint32_t SPIFFSFLOImage::ReadLE32(File* file)
{
  // Big-endian or unknown. Byte-by-byte read will perform reversal if needed.
  return file->read() | ((uint32_t)file->read() << 8) | ((uint32_t)file->read() << 16) | ((uint32_t)file->read() << 24);
}
