#pragma once

#include "outputs/outputs.h"

#include <Arduino.h>

// Forward declaration
struct CarStatus;

class CarActions
{
public:
  // Movimientos
  static void forward(OutputData& outputData, uint8_t speed);
  static void backward(OutputData& outputData, uint8_t speed);
  static void turnLeft(OutputData& outputData, uint8_t speed);
  static void turnRight(OutputData& outputData, uint8_t speed);
  static void freeStop(OutputData& outputData);
  static void forceStop(OutputData& outputData);

  // Servo
  static void setServoAngle(OutputData& outputData, uint8_t servoAngle);

  // LED
  static void setLedColor(OutputData& outputData, const String& color);

  // Estado global del coche
  static CarStatus getStatus();

private:
  static CarStatus currentStatus;
};

struct CarStatus
{
  String currentAction;
  uint8_t currentSpeed;
  uint8_t currentServoAngle;
  String currentLedColor;
  // unsigned long lastMovementTime;
};