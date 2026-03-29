#pragma once

#include <Arduino.h>

// Datos de salida (acciones a ejecutar)
struct OutputData
{
  uint8_t servoAngle;
  String ledColor;
  String action;
  uint8_t speed;
};