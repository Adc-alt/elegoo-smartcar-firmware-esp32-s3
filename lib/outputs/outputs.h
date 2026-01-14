#pragma once

#include <Arduino.h>

// Datos de salida (acciones a ejecutar)
struct OutputData
{
  int servoAngle;
  String ledColor;
  String action;
  uint8_t speed;
};