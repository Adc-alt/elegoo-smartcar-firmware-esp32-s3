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

// Comandos de alto nivel para el vehículo
enum class CarAction
{
  NONE,
  FORWARD,    // Ambos motores hacia adelante
  BACKWARD,   // Ambos motores hacia atrás
  TURN_LEFT,  // Motor derecho adelante, izquierdo parado
  TURN_RIGHT, // Motor izquierdo adelante, derecho parado
  FREE_STOP,
  FORCE_STOP,
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
  // Comando de movimiento del vehículo (alto nivel)
  CarAction vehicleAction = CarAction::NONE;
  uint8_t vehicleSpeed    = 0;

  // // Motores (bajo nivel - para compatibilidad o control fino)
  // struct
  // {
  //   MotorAction leftAction  = MotorAction::NONE;
  //   uint8_t leftSpeed       = 0;
  //   MotorAction rightAction = MotorAction::NONE;
  //   uint8_t rightSpeed      = 0;
  // } motors;

  // Servo
  bool servoHasCommand = false;
  uint8_t servoAngle   = 90;

  // LED
  bool ledHasCommand = false;
  LedColor ledColor  = LedColor::NONE;
};