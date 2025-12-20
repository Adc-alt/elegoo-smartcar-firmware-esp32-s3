// command_frame.h (en el ESP32)
#pragma once
#include <stdint.h>

enum class MotorAction
{
  NONE,
  FORWARD,
  REVERSE,
  FORCE_STOP,
  FREE_STOP
};

enum class LedColor
{
  NONE,
  BLACK,
  BLUE,
  RED,
  YELLOW,
  PURPLE,
  GREEN
};

struct CommandFrame
{
  // Motores
  struct
  {
    MotorAction leftAction  = MotorAction::NONE;
    uint8_t leftSpeed       = 0;
    MotorAction rightAction = MotorAction::NONE;
    uint8_t rightSpeed      = 0;
  } motors;

  // Servo
  bool servoHasCommand = false;
  uint8_t servoAngle   = 90;

  // LED
  bool ledHasCommand = false;
  LedColor ledColor  = LedColor::NONE;
};