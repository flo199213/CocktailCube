/*
 * Includes a spiffs bitmap image
 *
 * @author    Florian Stäblein
 * @date      2025/01/01
 * @copyright © 2025 Florian Stäblein
 */

//===============================================================
// Includes
//===============================================================
#include "SPIFFSBMPImage.h"

//===============================================================
// Constants
//===============================================================
static const char* TAG = "bmpimage";

//===============================================================
// Constructor
//===============================================================
SPIFFSBMPImage::SPIFFSBMPImage()
{
}

//===============================================================
// Destructor
//===============================================================
SPIFFSBMPImage::~SPIFFSBMPImage()
{
  Deallocate();
}

//===============================================================
// Allocates the internal buffer
//===============================================================
ImageReturnCode SPIFFSBMPImage::Allocate(String fileName)
{
  ESP_LOGI(TAG, "Allocating new SPIFFS image '%s'", fileName.c_str());
  
  // Check file name
  if (fileName.isEmpty())
  {
    return IMAGE_ERR_FILE_NOT_FOUND;
  }

  // Set file name
  _fileName = fileName;

  // Correct path if it does not start with "/"
  String filePath = _fileName;
  if (!filePath.startsWith("/"))
  {
    filePath = "/" + filePath;
  }

  // Open requested file on SPIFFS
  if (!(_file = SPIFFS.open(filePath, FILE_READ)))
  {
    return IMAGE_ERR_FILE_NOT_FOUND;
  }

  // Check if directory
  if (_file.isDirectory())
  {
    return IMAGE_ERR_FILE_NOT_FOUND;
  }

  // Parse BMP header. 0x4D42 (ASCII 'BM') is the Windows BMP signature.
  // There are other values possible in a .BMP file but these are super
  // esoteric (e.g. OS/2 struct bitmap array) and NOT supported here!
  if (ReadLE16(&_file) != 0x4D42)
  {
    _file.close();
    return IMAGE_ERR_FORMAT;
  }

  // BMP signature
  (void)ReadLE32(&_file);                         // Read & ignore file size (unreliable)
  (void)ReadLE32(&_file);                         // Read & ignore creator bytes
  uint32_t pixelDataPosition = ReadLE32(&_file);  // Start of image data in file
  
  // Read DIB header
  uint32_t headerSize = ReadLE32(&_file); // Indicates BMP version
  
  // Check header size
  if (headerSize < 40)
  {
    _file.close();
    return IMAGE_ERR_HEADER;
  }
  
  // Read bitmap size
  _width = ReadLE32(&_file);              // BMP width in pixels
  _height = ReadLE32(&_file);             // BMP height in pixels

  // Check for valid size
  if (_height <= 0 || _width <= 0)
  {
    _file.close();
    return IMAGE_ERR_FORMAT;
  }
  
  // Read planes and depth
  uint8_t planes = ReadLE16(&_file); // BMP planes (Should always be 1)
  uint8_t depth = ReadLE16(&_file);  // BMP bit depth (1, 4 and 8 bpp are indexed colors with color palette)

  // Check for correct color depth. We only accept 4 bit color palette
  if (planes != 1 || depth != 4)
  {
    _file.close();
    return IMAGE_ERR_DEPTH;
  }

  // Compression mode is present in later BMP versions (default = none)
  uint32_t compression = ReadLE32(&_file);            // Get compression
  (void)ReadLE32(&_file);                             // Raw bitmap data size; ignore
  (void)ReadLE32(&_file);                             // Horizontal resolution, ignore
  (void)ReadLE32(&_file);                             // Vertical resolution, ignore
  uint32_t paletteColorCount = ReadLE32(&_file);      // Number of colors in palette
  (void)ReadLE32(&_file);                             // Number of colors used
  
  // Only uncompressed bitmap with 16 color palettes are handled
  if (compression != 0 || paletteColorCount > PALETTE_COUNT)
  {
    _file.close();
    return IMAGE_ERR_TABLE;
  }
  
  // Move file position to the start of palette
  uint32_t palettePostion = BITMAPFILEHEADER_LENGTH + headerSize;
  if (_file.position() != palettePostion)
  {
    _file.seek(palettePostion);
  }
    
  // Calculate palette data byte size (16 colors, 4 bytes per color: Blue -> Green -> Red -> Reserved (0))
  size_t paletteByteSize = paletteColorCount * 4;
  
  // Read palette buffer
  uint8_t sourceBuffer[paletteByteSize] = {}; // BMP read buf (R+G+B+Reserved/pixel)

  // Read palette from file
  _file.read(sourceBuffer, sizeof sourceBuffer);

  // Set palette data buffer to zero
  memset(_bufferPalette, 0, PALETTE_COUNT);

  // For each color
  for (int16_t colorIndex = 0; colorIndex < PALETTE_COUNT; colorIndex++)
  {
    // Convert each pixel from BMP to 565 format, save in palette buffer
    uint8_t b = sourceBuffer[colorIndex * 4 + 0];
    uint8_t g = sourceBuffer[colorIndex * 4 + 1];
    uint8_t r = sourceBuffer[colorIndex * 4 + 2];
    _bufferPalette[colorIndex] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
  }
  
  // Move file position to the start of pixel data
  if (_file.position() != pixelDataPosition)
  {
    _file.seek(pixelDataPosition);
  }
    
  // BMP rows are padded (if needed) to 4-byte boundary
  _rowSize = ((depth * _width + 31) / 32) * 4; // > width if scanline padding
  
  // Calculate pixel data byte size
  size_t pixelDataByteSize = _rowSize * _height;

  // Check for enough heap (RAM) and allocate buffer
  if (pixelDataByteSize >= ESP.getMaxAllocHeap() ||
    !(_bufferPixelData = (uint8_t*)malloc(pixelDataByteSize)))
  {
    _file.close();
    return IMAGE_ERR_MALLOC;
  }

  // Set pixel data buffer to zero
  memset(_bufferPixelData, 0, pixelDataByteSize);
  ESP_LOGI(TAG, "New bitmap buffer allocated (Heap: %d / %d Bytes)", ESP.getFreeHeap(), ESP.getHeapSize());

  // Read palette from file
  size_t readCount = _file.read(_bufferPixelData, pixelDataByteSize);

  // Set valid flag
  _isValid = readCount == pixelDataByteSize;
  
  // Close file
  _file.close();

  return IMAGE_SUCCESS;
}

//===============================================================
// Deallocates the internal buffer
//===============================================================
void SPIFFSBMPImage::Deallocate()
{
  ESP_LOGI(TAG, "Deallocating SPIFFS image '%s'", _fileName.c_str());

  // Remove valid fag
  _isValid = false;

  // Set size to zero
  _width = 0;
  _height = 0;
  
  if (_file)
  {
    _file.close();
  }

  // Deallocate pixel data buffer
  if (_bufferPixelData)
  {
    free(_bufferPixelData);
    _bufferPixelData = NULL;
    ESP_LOGI(TAG, "Bitmap image buffer is free");
  }
  
  ESP_LOGI(TAG, "Image '%s' deleted (Heap: %d / %d Bytes)", _fileName.c_str(), ESP.getFreeHeap(), ESP.getHeapSize());
  _fileName = "";
}

//===============================================================
// Return a pixel at the requested position
//===============================================================
uint16_t SPIFFSBMPImage::GetPixel(int16_t x, int16_t y)
{
  if (!_bufferPixelData)
  {
    return 0;
  }
  
  // Calculate pixel byte index
  uint32_t pixelByteIndex = (_height - 1 - y) * _rowSize + x / 2;
  
  // Check range
  if (pixelByteIndex >= _rowSize * _height)
  {
    return 0;
  }
  
  // Read pixel byte and calculate palette index
  byte pixelByte = _bufferPixelData[pixelByteIndex];
  uint16_t paletteIndex = (x % 2 == 0) ? (pixelByte & 0xF0) >> 4 : (pixelByte & 0x0F);
    
  // Check range
  if (paletteIndex >= PALETTE_COUNT)
  {
    return 0;
  }
  
  return _bufferPalette[paletteIndex];
}

//===============================================================
// Draws the canvas on the tft
//===============================================================
bool SPIFFSBMPImage::Draw(int16_t x, int16_t y, Adafruit_SPITFT* tft, uint16_t shadowColor, bool asShadow)
{
  if (!_isValid)
  {
    return false;
  }
  
  // Write pixels
  tft->startWrite();
  uint16_t currentColor = 0;
  for (int16_t row = 0; row < _height; row++)
  {
    for (int16_t column = 0; column < _width; column++)
    {
      currentColor = GetPixel(column, row);

      if (currentColor != TRANSPARENCY_COLOR)
      {
        tft->writePixel(x + column, y + row, asShadow ? shadowColor : currentColor);
      }
    }
  }
  tft->endWrite();
  
  return true;
}

//===============================================================
// Clears the difference between two images
//===============================================================
void SPIFFSBMPImage::ClearDiff(int16_t x0, int16_t y0, int16_t x1, int16_t y1, SPIFFSBMPImage* otherImage, Adafruit_SPITFT* tft, uint16_t clearColor)
{
  if (otherImage == NULL)
  {
    return;
  }

  if (!_isValid ||
    !otherImage->IsValid())
  {
    return;
  }
 
  int16_t otherHeight = otherImage->_height;
  int16_t otherWidth = otherImage->_width;

  // Write pixels
  tft->startWrite();
  for (int16_t row = 0; row < _height; row++)
  {
    for (int16_t column = 0; column < _width; column++)
    {
      uint16_t currentColor = GetPixel(column, row);
      
      // Calculate other indexes
      int16_t otherColumn = column - (x1 - x0);
      int16_t otherRow = row - (y1 - y0);

      uint16_t otherColor = TRANSPARENCY_COLOR + 1; // Use color != transparencyColor
      if (otherColumn > 0 && otherColumn < otherWidth &&
        otherRow > 0 && otherRow < otherHeight)
      {
        otherColor = otherImage->GetPixel(otherColumn, otherRow);
      }

      // Clear color, if current color is not transparent and other color is transparent (must be reset)
      if (currentColor != TRANSPARENCY_COLOR &&
        otherColor == TRANSPARENCY_COLOR)
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
void SPIFFSBMPImage::Move(int16_t x0, int16_t y0, int16_t x1, int16_t y1, Adafruit_SPITFT* tft, uint16_t clearColor, bool onlyClear)
{
  if (!_isValid)
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
      uint16_t colorOld = GetPixel(column, row);

      // Calculate new color indexes
      int16_t newColumn = column - (x1 - x0);
      int16_t newRow = row - (y1 - y0);
      
      // Get new color value
      uint16_t colorNew = TRANSPARENCY_COLOR;
      if (newColumn > 0 && newColumn < _width &&
        newRow > 0 && newRow < _height)
      {
        colorNew = GetPixel(newColumn, newRow);
      }
      
      // Reset pixel only if the color would be transparent and old color was not
      if (colorOld != TRANSPARENCY_COLOR &&
        colorNew == TRANSPARENCY_COLOR)
      {
        tft->writePixel(x0 + column, y0 + row, clearColor);
      }
    }
  }
  tft->endWrite();
  
  if (!onlyClear)
  {
    // Draw new (moved) image
    Draw(x1, y1, tft);
  }
}

//===============================================================
// Print error code string to stream
//===============================================================
String SPIFFSBMPImage::PrintStatus(ImageReturnCode stat)
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
    return String("Not a supported image format.");
  }
  else if (stat == IMAGE_ERR_HEADER)
  {
    return String("Does not contain header.");
  }
  else if (stat == IMAGE_ERR_DEPTH)
  {
    return String("Not supported color depth.");
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
uint16_t SPIFFSBMPImage::ReadLE16(File* file)
{
  // Big-endian or unknown. Byte-by-byte read will perform reversal if needed.
  return file->read() | ((uint16_t)file->read() << 8);
}

//===============================================================
// Reads a little-endian 32-bit unsigned value from currently-
// open File, converting if necessary to the microcontroller's
// native endianism. (BMP files use little-endian values.)
//===============================================================
uint32_t SPIFFSBMPImage::ReadLE32(File* file)
{
  // Big-endian or unknown. Byte-by-byte read will perform reversal if needed.
  return file->read() | ((uint32_t)file->read() << 8) | ((uint32_t)file->read() << 16) | ((uint32_t)file->read() << 24);
}
