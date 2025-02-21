/*
 * Includes a spiffs bitmap image
 *
 * @author    Florian Stäblein
 * @date      2025/01/01
 * @copyright © 2025 Florian Stäblein
 */
 
#ifndef SPIFFSBMPIMAGE_H
#define SPIFFSBMPIMAGE_H

//===============================================================
// Inlcudes
//===============================================================
#include <Arduino.h>
#include <cmath>
#include <SPIFFS.h>
#include <Adafruit_SPITFT.h>

//===============================================================
// Defines
//===============================================================
#define BITMAPFILEHEADER_LENGTH   14
#define PALETTE_COUNT             16
#define TRANSPARENCY_COLOR        0x07E0  // 100% green in 565 format means transparent

//===============================================================
// Enums
//===============================================================
enum ImageReturnCode
{
  IMAGE_SUCCESS,            // Successful load
  IMAGE_ERR_FILE_NOT_FOUND, // Could not open file
  IMAGE_ERR_FORMAT,         // Not a supported image format
  IMAGE_ERR_HEADER,         // Does not contain header
  IMAGE_ERR_DEPTH,          // Not supported color depth
  IMAGE_ERR_TABLE,          // Does not contain color table
  IMAGE_ERR_MALLOC,         // Could not allocate image
  IMAGE_ERR_PIXELDATA       // Not enough pixel data read
};

//===============================================================
// SPIFFS bitmap image class
//===============================================================
class SPIFFSBMPImage
{
  public:
    // Constructor
    SPIFFSBMPImage();
    
    // Destructor
    ~SPIFFSBMPImage();

    // Allocates the internal buffer
    ImageReturnCode Allocate(String fileName);

    // Deallocates the internal buffer
    void Deallocate();

    // Return the height of the image
    int16_t Height() const { return _height; }

    // Return the width of the image
    int16_t Width() const { return _width; }

    // Return the transparency color of the image
    uint16_t TransparencyColor() const { return TRANSPARENCY_COLOR; }

    // Return the valid state of the image
    int16_t IsValid() const { return _isValid; }
    
    // Return a pixel at the requested position
    uint16_t GetPixel(int16_t x, int16_t y);
    
    // Draws the image on the tft
    bool Draw(int16_t x, int16_t y, Adafruit_SPITFT* tft, uint16_t shadowColor = 0, bool asShadow = false);

    // Clears the difference between two images
    void ClearDiff(int16_t x0, int16_t y0, int16_t x1, int16_t y1, SPIFFSBMPImage* otherImage, Adafruit_SPITFT* tft, uint16_t clearColor);

    // Moves the image on the tft
    void Move(int16_t x0, int16_t y0, int16_t x1, int16_t y1, Adafruit_SPITFT* tft, uint16_t clearColor, bool onlyClear = false);

    // Print error code string to stream
    String PrintStatus(ImageReturnCode stat);

  private:
    // File object for reading image data
    String _fileName;
    File _file;

    // Image size
    int32_t _width = 0;
    int32_t _height = 0;
    
    // Buffer variables
    uint32_t _rowSize = 0;
    
    // Buffer which stores the palette
    uint16_t _bufferPalette[PALETTE_COUNT] = {};

    // Buffer which stores the pixel data
    uint8_t* _bufferPixelData = NULL;

    // True if the image is valid loaded
    bool _isValid = false;

    // Reads a little-endian 16-bit
    uint16_t ReadLE16(File* file);

    // Reads a little-endian 32-bit
    uint32_t ReadLE32(File* file);
};

#endif
