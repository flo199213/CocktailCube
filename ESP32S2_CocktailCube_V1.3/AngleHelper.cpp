/**
 * Includes all angle functions
 *
 * @author    Florian Staeblein
 * @date      2024/01/28
 * @copyright © 2024 Florian Staeblein
 */

//===============================================================
// Includes
//===============================================================
#include "AngleHelper.h"

//===============================================================
// Increments the value by the angle distance given
//===============================================================
void IncrementAngle(int16_t* value, int16_t nextBorder, int16_t previousBorder, int16_t angleDistance_Degrees)
{
  bool clockwise = angleDistance_Degrees > 0;
  
  for (uint16_t index = 0; index < abs(angleDistance_Degrees); index++)
  {
    int16_t newValue = Move360(*value, clockwise ? 1 : -1);
    int16_t distanceToNextBorder_Degrees = GetDistanceDegrees(newValue, clockwise ? nextBorder : previousBorder);
    distanceToNextBorder_Degrees = clockwise ? distanceToNextBorder_Degrees : 360 - distanceToNextBorder_Degrees;
    
    if (distanceToNextBorder_Degrees >= MINANGLE_DEGREES)
    {
      *value = newValue;
    }
    else
    {
      break;
    }
  }
}

//===============================================================
// Moves a value in 360 degrees space around the specified distance
//===============================================================
int16_t Move360(int16_t value, int16_t distance)
{
  int newValue = value + distance;
  if (newValue >= 360)
  {
    newValue -= 360;
  }
  if (newValue < 0)
  {
    newValue += 360;
  }
  return newValue;
}

//===============================================================
// Return the clockwise distance between two angles in an 360°
// space
//===============================================================
int16_t GetDistanceDegrees(int16_t startAngle, int16_t stopAngle)
{
  // Set inital variables
  int16_t distance = 0;
  int16_t runAngle = startAngle;
  
  // Calculate distance between two angles clock wise with regard to an overflow over 0/360°
  while(runAngle != stopAngle)
  {
    runAngle = Move360(runAngle, 1);
    distance++;
  }
 
  return distance;
}
