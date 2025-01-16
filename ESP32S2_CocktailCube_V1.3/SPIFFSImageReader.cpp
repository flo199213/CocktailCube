/**
 * Includes all spiffs image functions
 *
 * @author    Florian Stäblein
 * @date      2025/01/01
 * @copyright © 2025 Florian Stäblein
 */

//===============================================================
// Includes
//===============================================================
#include "SPIFFSImageReader.h"

//===============================================================
// Constants
//===============================================================
static const char* TAG = "spiffsreader";

//===============================================================
// Defines
//===============================================================
#define BUFPIXELS 200

//===============================================================
// Constructor
//===============================================================
SPIFFSImageReader::SPIFFSImageReader()
{
}

//===============================================================
// Destructor
//===============================================================
SPIFFSImageReader::~SPIFFSImageReader()
{
  if (_file)
  {
    _file.close();
  }
}

//===============================================================
// Loads BMP image file from SPIFFS into RAM
//===============================================================
ImageReturnCode SPIFFSImageReader::LoadBMP(String filename, SPIFFSImage* img)
{
  ESP_LOGI(TAG, "Loading image '%s'", filename.c_str());

  // Correct path
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

  // Parse BMP header. 0x4D42 (ASCII 'BM') is the Windows BMP signature.
  // There are other values possible in a .BMP file but these are super
  // esoteric (e.g. OS/2 struct bitmap array) and NOT supported here!
  if (ReadLE16() != 0x4D42)
  {
    _file.close();
    return IMAGE_ERR_FORMAT;
  }

  // BMP signature
  (void)ReadLE32();             // Read & ignore file size
  (void)ReadLE32();             // Read & ignore creator bytes
  uint32_t offset = ReadLE32(); // Start of image data in file
  
  // Read DIB header
  uint32_t headerSize = ReadLE32(); // Indicates BMP version
  int16_t bmpWidth = ReadLE32();    // BMP width in pixels
  int16_t bmpHeight = ReadLE32();   // BMP height in pixels

  // If bmpHeight is negative, image is in top-down order.
  // This is not canon but has been observed in the wild
  bool flip = true;  // BMP is stored bottom-to-top
  if (bmpHeight < 0)
  {
    bmpHeight = -bmpHeight;
    flip = false;   // BMP is stored top-to-bottom
  }
                          
  uint8_t planes = ReadLE16(); // BMP planes
  uint8_t depth = ReadLE16();  // BMP bit depth (Bits per pixel)

  // Check for correct color depth
  if (depth != 24)
  {
    _file.close();
    return IMAGE_ERR_FORMAT;
  }

  // Compression mode is present in later BMP versions (default = none)
  uint32_t compression = 0; // BMP compression mode
  if (headerSize > 12)
  {
    compression = ReadLE32();
    (void)ReadLE32();    // Raw bitmap data size; ignore
    (void)ReadLE32();    // Horizontal resolution, ignore
    (void)ReadLE32();    // Vertical resolution, ignore
    (void)ReadLE32();    // Number of colors in palette, ignore
    (void)ReadLE32();    // Number of colors used, ignore
    // File position should now be at start of palette (if present)
  }

  // Only uncompressed is handled
  if (planes != 1 || compression != 0)
  {
    _file.close();
    return IMAGE_ERR_FORMAT;
  }

  // Loading to RAM
  // Allocate 16-bit buffer image
  if (!img->Allocate(bmpWidth, bmpHeight))
  {
    _file.close();
    return IMAGE_ERR_MALLOC;
  }

  // BMP rows are padded (if needed) to 4-byte boundary
  uint32_t rowSize = ((depth * bmpWidth + 31) / 32) * 4; // > bmpWidth if scanline padding

  // Read file buffer
  uint8_t sdbuf[3 * BUFPIXELS] = {}; // BMP read buf (R+G+B/pixel)
  uint16_t srcidx = sizeof sdbuf;    // Source buffer pointer
  uint32_t bmpPos = 0;               // Next pixel position in file

  // Get working buffer and working buffer pointer
  uint32_t destidx = 0;
  uint16_t* dest = img->GetBuffer();

  // For each scanline...
  for (int16_t row = 0; row < bmpHeight; row++)
  {
    // yield() for ESP32
    yield();

    // Seek to start of scan line.  It might seem labor-intensive
    // to be doing this on every line, but this method covers a
    // lot of gritty details like cropping, flip and scanline
    // padding. Also, the seek only takes place if the file
    // position actually needs to change (avoids a lot of cluster
    // math in SD library).
    if (flip) // Bitmap is stored bottom-to-top order (normal BMP)
    {
      bmpPos = offset + (bmpHeight - 1 - row) * rowSize;
    }
    else // Bitmap is stored top-to-bottom
    {
      bmpPos = offset + row * rowSize;
    }
    
    // Need seek?
    if (_file.position() != bmpPos)
    { 
      _file.seek(bmpPos);

      // Force buffer reload
      srcidx = sizeof sdbuf;
    }

    // For each pixel
    for (int16_t column = 0; column < bmpWidth; column++)
    {
      // Time to load more?
      if (srcidx >= sizeof sdbuf)
      {
        // Read file from SPIFFS
        _file.read(sdbuf, sizeof sdbuf);

        // Reset bmp buf index
        srcidx = 0;
      }

      // Convert each pixel from BMP to 565 format, save in dest
      uint8_t b = sdbuf[srcidx++];
      uint8_t g = sdbuf[srcidx++];
      uint8_t r = sdbuf[srcidx++];
      dest[destidx++] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);

    }
  }

  // Close file
  _file.close();

  return IMAGE_SUCCESS;
}

//===============================================================
// Reads a little-endian 16-bit unsigned value from currently-
// open File, converting if necessary to the microcontroller's
// native endianism. (BMP files use little-endian values.)
//===============================================================
uint16_t SPIFFSImageReader::ReadLE16()
{
  // Big-endian or unknown. Byte-by-byte read will perform reversal if needed.
  return _file.read() | ((uint16_t)_file.read() << 8);
}

//===============================================================
// Reads a little-endian 32-bit unsigned value from currently-
// open File, converting if necessary to the microcontroller's
// native endianism. (BMP files use little-endian values.)
//===============================================================
uint32_t SPIFFSImageReader::ReadLE32()
{
  // Big-endian or unknown. Byte-by-byte read will perform reversal if needed.
  return _file.read() | ((uint32_t)_file.read() << 8) | ((uint32_t)_file.read() << 16) | ((uint32_t)_file.read() << 24);
}

//===============================================================
// Print error code string to stream
//===============================================================
String SPIFFSImageReader::PrintStatus(ImageReturnCode stat)
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
    return String("Not a supported BMP variant.");
  }
  else if (stat == IMAGE_ERR_MALLOC)
  {
    return String("Malloc failed (insufficient RAM).");
  }

  return "Unknown";
}
