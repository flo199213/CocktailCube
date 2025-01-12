/**
 * Includes all angle functions
 *
 * @author    Florian Staeblein
 * @date      2024/01/28
 * @copyright © 2024 Florian Staeblein
 */
 
#ifndef ANGLEHELPER_H
#define ANGLEHELPER_H

//===============================================================
// Includes
//===============================================================
#include <Arduino.h>
#include "Config.h"

//===============================================================
// Defines
//===============================================================
#define STEPANGLE_DEGREES       3     // Angle which will be used for one encoder step
#define MINANGLE_DEGREES        6     // Minimum distance angle between two angle settings

//===============================================================
// Declarations
//===============================================================

// Increments the value by the angle distance given
void IncrementAngle(int16_t* value, int16_t nextBorder, int16_t previousBorder, int16_t angleDistance_Degrees);

// Moves a value in 360 degrees space around the specified positive or negative distance
int16_t Move360(int16_t value, int16_t distance);

// Return the clockwise distance between two angles in an 360° space
int16_t GetDistanceDegrees(int16_t startAngle, int16_t stopAngle);

#endif
