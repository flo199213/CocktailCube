/**
 * Includes all spiffs image functions
 *
 * @author    Florian Staeblein
 * @date      2024/01/28
 * @copyright © 2024 Florian Staeblein
 */

//===============================================================
// Includes
//===============================================================
#include "SPIFFSImageReader.h"

//===============================================================
// Defines
//===============================================================
#define BUFPIXELS 200

//===============================================================
// Constructor
//===============================================================
SPIFFSImage::SPIFFSImage()
{
  Canvas16 = NULL;
}

//===============================================================
// Destructor
//===============================================================
SPIFFSImage::~SPIFFSImage()
{
  Dealloc();
}

//===============================================================
// Deallocates memory associated with SPIFFSImage object and
// resets member variables to 'empty' state
//===============================================================
void SPIFFSImage::Dealloc()
{
  if (Canvas16)
  {
    delete Canvas16;
    Canvas16 = NULL;
  }
}

//===============================================================
// Draws the canvas on the tft
//===============================================================
void SPIFFSImage::Draw(int16_t x, int16_t y, Adafruit_SPITFT *tft, uint16_t transparencyColor)
{
  uint16_t* buffer = Canvas16->getBuffer();
  int16_t height = Canvas16->height();
  int16_t width = Canvas16->width();

  // Write pixels
  tft->startWrite();
  for (int16_t row = 0; row < height; row++)
  {
    for (int16_t column = 0; column < width; column++)
    {
      uint16_t currentPixel = buffer[row * width + column];
      if (currentPixel != transparencyColor)
      {
        tft->writePixel(x + column, y + row, currentPixel);
      }
    }
  }
  tft->endWrite();
}

//===============================================================
// Moves the canvas on the tft
//===============================================================
void SPIFFSImage::Move(int16_t x0, int16_t y0, int16_t x1, int16_t y1, Adafruit_SPITFT *tft, uint16_t clearColor, uint16_t transparencyColor)
{
  uint16_t* buffer = Canvas16->getBuffer();
  int16_t height = Canvas16->height();
  int16_t width = Canvas16->width();

  // Clear old image (only diff to new one, to avoid flickering)
  tft->startWrite();
  for (int16_t row = 0; row < height; row++)
  {
    for (int16_t column = 0; column < width; column++)
    {
      // Get old color value
      uint16_t colorOld = buffer[row * width + column];

      // Calculate new color indexes
      int16_t newColumn = column - (x1 - x0);
      int16_t newRow = row - (y1 - y0);
      
      // Get new color value
      uint16_t colorNew = transparencyColor;
      if (newColumn > 0 && newColumn < width &&
        newRow > 0 && newRow < height)
      {
        colorNew = buffer[newRow * width + newColumn];
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

  // Draw new (moved) image
  Draw(x1, y1, tft, transparencyColor);
}

//===============================================================
// Return a pixel at the requested position
//===============================================================
uint16_t SPIFFSImage::GetPixel(int16_t x, int16_t y)
{
  uint16_t* buffer = Canvas16->getBuffer();
  int16_t height = Canvas16->height();
  int16_t width = Canvas16->width();

  int16_t index = y * width + x;

  if (index < width * height)
  {
    return buffer[index];
  }
  
  return 0;
}

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
ImageReturnCode SPIFFSImageReader::LoadBMP(const char *filename, SPIFFSImage *img)
{
  uint16_t *dest;                             // Working buffer
  uint32_t destidx = 0;                       // Working buffer pointer
  uint32_t offset;                            // Start of image data in file
  uint32_t headerSize;                        // Indicates BMP version
  int16_t bmpWidth;                           // BMP width in pixels
  int16_t bmpHeight;                          // BMP height in pixels
  uint8_t planes;                             // BMP planes
  uint8_t depth;                              // BMP bit depth
  uint32_t compression = 0;                   // BMP compression mode
  uint32_t rowSize;                           // >bmpWidth if scanline padding
  uint8_t sdbuf[3 * BUFPIXELS] = {};          // BMP read buf (R+G+B/pixel)
  uint16_t srcidx = sizeof sdbuf;             // Source buffer pointer
  bool flip = true;                           // BMP is stored bottom-to-top
  uint32_t bmpPos = 0;                        // Next pixel position in file
  int16_t row;                                // Current pixel position row
  int16_t column;                             // Current pixel position column
  uint8_t r;                                  // Current pixel color red
  uint8_t g;                                  // Current pixel color green
  uint8_t b;                                  // Current pixel color blue
  
  // If an SPIFFSImage object is passed and currently contains anything,
  // free its contents as it's about to be overwritten with new stuff
  img->Dealloc();

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
  (void)ReadLE32();         // Read & ignore file size
  (void)ReadLE32();         // Read & ignore creator bytes
  offset = ReadLE32();      // Start of image data
  
  // Read DIB header
  headerSize = ReadLE32();
  bmpWidth = ReadLE32();
  bmpHeight = ReadLE32();

  // If bmpHeight is negative, image is in top-down order.
  // This is not canon but has been observed in the wild
  if (bmpHeight < 0)
  {
    bmpHeight = -bmpHeight;
    flip = false;
  }
  planes = ReadLE16();
  depth = ReadLE16(); // Bits per pixel

  // Check for correct color depth
  if (depth != 24)
  {
    _file.close();
    return IMAGE_ERR_FORMAT;
  }

  // Compression mode is present in later BMP versions (default = none)
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

  // BMP rows are padded (if needed) to 4-byte boundary
  rowSize = ((depth * bmpWidth + 31) / 32) * 4;

  // Loading to RAM -- allocate GFX 16-bit canvas type
  // Check for alloc OK
  if (!(img->Canvas16 = new GFXcanvas16(bmpWidth, bmpHeight)))
  {
    _file.close();
    return IMAGE_ERR_MALLOC;
  }

  // Get working buffer pointer
  dest = img->Canvas16->getBuffer();

  // For each scanline...
  for (row = 0; row < bmpHeight; row++)
  {
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

    // For each pixel...
    for (column = 0; column < bmpWidth; column++)
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
      b = sdbuf[srcidx++];
      g = sdbuf[srcidx++];
      r = sdbuf[srcidx++];
      dest[destidx++] = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);

    } // end column loop
  } // end row loop

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
