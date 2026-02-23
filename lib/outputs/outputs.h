#pragma once

#include <Arduino.h>

// Datos de salida (acciones a ejecutar)
struct OutputData
{
  uint8_t servoAngle;
  String ledColor;
  // // Formato antiguo: mismo action/speed para ambos motores
  // String action;
  // uint8_t speed;
  // Formato diferencial: solo se usa si useDifferentialMotors == true
  // bool useDifferentialMotors = false;
  String leftAction;
  uint8_t leftSpeed = 0;
  String rightAction;
  uint8_t rightSpeed = 0;
};