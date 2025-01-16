/**
 * Includes all spiffs image functions
 *
 * @author    Florian Stäblein
 * @date      2025/01/01
 * @copyright © 2025 Florian Stäblein
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
#include "SPIFFSImage.h"

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
    ImageReturnCode LoadBMP(String filename, SPIFFSImage* img);

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
