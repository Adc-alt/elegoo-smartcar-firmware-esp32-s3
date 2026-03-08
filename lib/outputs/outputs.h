#pragma once

#include <Arduino.h>

// Datos de salida (acciones a ejecutar)
struct OutputData
{
  uint8_t servoAngle;
  String ledColor;
  String leftAction;
  uint8_t leftSpeed;
  String rightAction;
  uint8_t rightSpeed;
};