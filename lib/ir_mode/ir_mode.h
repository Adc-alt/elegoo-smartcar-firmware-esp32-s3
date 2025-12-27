#pragma once
#include "../command_frame/command_frame.h"
#include "../command_sender/command_sender.h"
#include "../telemetry_frame/telemetry_frame.h"

#include <Arduino.h>

// Estados de la máquina de estados IR
enum class IrStatus
{
  IDLE,           // Estado inactivo
  FORWARD,        // Avanzar
  BACKWARD,       // Retroceder
  LEFT,           // Girar izquierda
  RIGHT,          // Girar derecha
  STOP,           // Parar
  SERVO_CENTERED, // Servo centrado
  SERVO_LEFT,     // Servo izquierda
  SERVO_RIGHT     // Servo derecha
};

// Constantes de configuración
const uint8_t IR_MOTOR_SPEED       = 80;
const uint8_t IR_SERVO_CENTER      = 90;
const uint8_t IR_SERVO_LEFT_ANGLE  = 0;
const uint8_t IR_SERVO_RIGHT_ANGLE = 180;

class IrMode
{
public:
  explicit IrMode(CommandSender& commandSender);

  // Procesar telemetría y actualizar estado/enviar comandos
  void handle(const TelemetryFrame& telemetryFrame);

  // Obtener estado actual
  IrStatus getStatus() const
  {
    return status;
  }

private:
  // Actualizar estado basándose en el comando IR recibido
  void updateStateFromIrData(const String& irData);

  // Enviar comando basado en el estado actual
  void sendCommandForCurrentState();

  void forward();
  void backward();
  void left();
  void right();
  void stop();
  void servoCentered();
  void servoLeft();
  void servoRight();

  // Estado actual
  IrStatus status = IrStatus::IDLE;
  CommandSender& commandSender;
  String lastIrData = ""; // Para evitar procesar el mismo comando múltiples veces
};
