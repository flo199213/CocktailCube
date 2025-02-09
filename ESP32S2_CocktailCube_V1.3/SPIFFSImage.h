/*
 * Includes a spiffs FLO image
 *
 * @author    Florian Stäblein
 * @date      2025/01/01
 * @copyright © 2025 Florian Stäblein
 */
 
#ifndef SPIFFSIMAGE_H
#define SPIFFSIMAGE_H

//===============================================================
// Inlcudes
//===============================================================
#include <Arduino.h>
#include <cmath>
#include <SPIFFS.h>
#include <Adafruit_SPITFT.h>
#include "Config.h"

//===============================================================
// Defines
//===============================================================
#define MAX_TABLE_SIZE    16

//===============================================================
// Enums
//===============================================================
enum ImageReturnCode
{
  IMAGE_SUCCESS,            // Successful load
  IMAGE_ERR_FILE_NOT_FOUND, // Could not open file
  IMAGE_ERR_FORMAT,         // Not a supported image format
  IMAGE_ERR_HEADER,         // Does not contain FLO header
  IMAGE_ERR_TABLE,          // Does not contain color table
  IMAGE_ERR_MALLOC,         // Could not allocate image
  IMAGE_ERR_PIXELDATA       // Not enough pixel data read
};

//===============================================================
// Structs
//===============================================================
struct FLOHeader
{
  uint16_t width;
  uint16_t height;
  uint16_t colorTable[MAX_TABLE_SIZE];
};

//===============================================================
// SPIFFS image class
//===============================================================
class SPIFFSImage
{
  public:
    // Constructor
    SPIFFSImage();
    
    // Destructor
    ~SPIFFSImage();

    // Allocates the internal buffer
    ImageReturnCode Allocate(String filename);

    // Deallocates the internal buffer
    void Deallocate();

    // Return the height of the image
    int16_t Height() const { return _header.height; }

    // Return the width of the image
    int16_t Width() const { return _header.width; }

    // Return the transparency color of the image
    uint16_t TransparencyColor() const { return _header.colorTable[0]; }

    // Return the valid state of the image
    int16_t IsValid() const { return _isValid; }
        
    // Return a pixel at the requested position
    uint16_t GetPixel(int16_t x, int16_t y);
    
    // Draws the image on the tft
    void Draw(int16_t x, int16_t y, Adafruit_SPITFT* tft, uint16_t shadowColor = 0, bool asShadow = false);

    // Clears the difference between two images
    void ClearDiff(int16_t x0, int16_t y0, int16_t x1, int16_t y1, SPIFFSImage* otherImage, Adafruit_SPITFT* tft, uint16_t clearColor);

    // Moves the image on the tft
    void Move(int16_t x0, int16_t y0, int16_t x1, int16_t y1, Adafruit_SPITFT* tft, uint16_t clearColor, bool onlyClear = false);

    // Print error code string to stream
    String PrintStatus(ImageReturnCode stat);

  private:
    // File object for reading image data
    File _file;

    // Image settings header
    FLOHeader _header;

    // Buffer which stores the pixel data
    uint8_t* _buffer = NULL;

    // True if the image is valid loaded
    bool _isValid = false;

    // Reads a little-endian 16-bit
    uint16_t ReadLE16(File* file);

    // Reads a little-endian 32-bit
    uint32_t ReadLE32(File* file);
};

#endif
