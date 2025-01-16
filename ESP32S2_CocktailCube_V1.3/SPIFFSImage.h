/**
 * Includes a spiffs image
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
#include <SPIFFS.h>
#include <Adafruit_SPITFT.h>
#include "Config.h"

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
    bool Allocate(int16_t width, int16_t height);

    // Deallocates the internal buffer
    void Deallocate();

    // Return the height of the image
    int16_t Height() const { return _height; }

    // Return the width of the image
    int16_t Width() const { return _width; }

    // Returns the internal buffer
    uint16_t* GetBuffer() const { return _buffer; }
    
    // Draws the canvas on the tft
    void Draw(int16_t x, int16_t y, Adafruit_SPITFT* tft, uint16_t transparencyColor, uint16_t shadowColor = 0, bool asShadow = false);

    // Clears the difference between two images
    void ClearDiff(int16_t x0, int16_t y0, int16_t x1, int16_t y1, SPIFFSImage* otherImage, Adafruit_SPITFT* tft, uint16_t transparencyColor, uint16_t clearColor);

    // Moves the canvas on the tft
    void Move(int16_t x0, int16_t y0, int16_t x1, int16_t y1, Adafruit_SPITFT* tft, uint16_t clearColor, uint16_t transparencyColor, bool onlyClear = false);

    // Return a pixel at the requested position
    uint16_t GetPixel(int16_t x, int16_t y);

  private:
    // Image size
    int16_t _width = 0;
    int16_t _height = 0;

    // Buffer which stores the pixel data
    uint16_t* _buffer;
};

#endif
