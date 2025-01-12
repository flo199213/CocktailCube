/**
 * Includes all spiffs image functions
 *
 * @author    Florian Staeblein
 * @date      2024/01/28
 * @copyright © 2024 Florian Staeblein
 */
 
#ifndef SPIFFSIMAGEREADER_H
#define SPIFFSIMAGEREADER_H

//===============================================================
// Inlcudes
//===============================================================
#include <Arduino.h>
#include <SPIFFS.h>
#include <Adafruit_SPITFT.h>
#include "Config.h"

//===============================================================
// Enums
//===============================================================
enum ImageReturnCode
{
  IMAGE_SUCCESS,            // Successful load
  IMAGE_ERR_FILE_NOT_FOUND, // Could not open file
  IMAGE_ERR_FORMAT,         // Not a supported image format
  IMAGE_ERR_MALLOC          // Could not allocate image
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

    // Return the height of the image
    int16_t Height() { return Canvas16->height(); }

    // Return the width of the image
    int16_t Width() { return Canvas16->width(); }
    
    // Draws the canvas on the tft
    void Draw(int16_t x, int16_t y, Adafruit_SPITFT *tft, uint16_t transparencyColor);

    // Moves the canvas on the tft
    void Move(int16_t x0, int16_t y0, int16_t x1, int16_t y1, Adafruit_SPITFT *tft, uint16_t clearColor, uint16_t transparencyColor);

    // Return a pixel at the requested position
    uint16_t GetPixel(int16_t x, int16_t y);

  private:
    // Canvas which stores the pixel data
    GFXcanvas16* Canvas16;

    // Free/deinitializes variables
    void Dealloc();      

    // Loading occurs here
    friend class SPIFFSImageReader;
};

//===============================================================
// SPIFFS image reader class
//===============================================================
class SPIFFSImageReader
{
  public:
    // Constructor
    SPIFFSImageReader();
    
    // Destructor
    ~SPIFFSImageReader();

    // Loads BMP image file from SPIFFS into RAM
    ImageReturnCode LoadBMP(const char *filename, SPIFFSImage *img);

    // Print error code string to stream
    String PrintStatus(ImageReturnCode stat);

  private:
    // File object for reading image data
    File _file;

    // Reads a little-endian 16-bit
    uint16_t ReadLE16();

    // Reads a little-endian 32-bit
    uint32_t ReadLE32();
};

#endif
