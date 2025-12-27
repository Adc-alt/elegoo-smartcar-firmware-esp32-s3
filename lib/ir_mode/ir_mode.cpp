#include "ir_mode.h"

// Tabla de mapeo: string -> estado
struct IrCommandMapping
{
  const char* command;
  IrStatus status;
};

// Tabla de comandos IR a estados (ordenada alfabéticamente para búsqueda más eficiente)
static const IrCommandMapping IR_COMMAND_MAP[] = {{"backward", IrStatus::BACKWARD},
                                                  {"forward", IrStatus::FORWARD},
                                                  {"left", IrStatus::LEFT},
                                                  {"reverse", IrStatus::BACKWARD}, // Alias para backward
                                                  {"right", IrStatus::RIGHT},
                                                  {"servo_center", IrStatus::SERVO_CENTERED},
                                                  {"servo_centered", IrStatus::SERVO_CENTERED},
                                                  {"servo_left", IrStatus::SERVO_LEFT},
                                                  {"servo_right", IrStatus::SERVO_RIGHT},
                                                  {"stop", IrStatus::STOP}};

// Para no hardcodear el tamaño de la tabla de mapeo se usa sizeof
// y se divide tamaño total/ tamaño de un elemento = numero de elementos = elementos de la tabla
// static const size_t IR_COMMAND_MAP_SIZE = sizeof(IR_COMMAND_MAP) / sizeof(IR_COMMAND_MAP[0]);

IrMode::IrMode(CommandSender& commandSender) : commandSender(commandSender)
{
}

void IrMode::handle(const TelemetryFrame& telemetryFrame)
{
  // Solo procesar si hay datos nuevos del IR
  if (!telemetryFrame.ir_new)
  {
    return;
  }

  // Evitar procesar el mismo comando múltiples veces
  if (telemetryFrame.ir_data == lastIrData)
  {
    return;
  }

  lastIrData = telemetryFrame.ir_data;

  String irData = telemetryFrame.ir_data;
  irData.toLowerCase();

  Serial.print("[IR_MODE] Comando recibido: ");
  Serial.println(irData);

  // Actualizar estado usando la tabla de mapeo
  updateStateFromIrData(irData);

  // Enviar comando basado en el estado actual
  sendCommandForCurrentState();
}

void IrMode::updateStateFromIrData(const String& irData)
{
  // Buscar el comando en la tabla de mapeo
  for (size_t i = 0; i < 10; i++)
  {
    if (irData == IR_COMMAND_MAP[i].command)
    {
      status = IR_COMMAND_MAP[i].status;
      //       Serial.print("  -> Estado: ");
      //       Serial.println(static_cast<int>(status));
      return;
    }
  }

  // Comando no encontrado
  Serial.print("  -> Comando desconocido: ");
  Serial.println(irData);
}

void IrMode::sendCommandForCurrentState()
{
  switch (status)
  {
    case IrStatus::IDLE:
      break;

    case IrStatus::FORWARD:
      forward();
      break;

    case IrStatus::BACKWARD:
      backward();
      break;

    case IrStatus::LEFT:
      left();
      break;

    case IrStatus::RIGHT:
      right();
      break;

    case IrStatus::STOP:
      stop();
      break;

    case IrStatus::SERVO_CENTERED:
      servoCentered();
      break;

    case IrStatus::SERVO_LEFT:
      servoLeft();
      break;

    case IrStatus::SERVO_RIGHT:
      servoRight();
      break;
  }
}

void IrMode::forward()
{
  CommandFrame frame;
  frame.vehicleAction = CarAction::FORWARD;
  frame.vehicleSpeed  = IR_MOTOR_SPEED;
  commandSender.send(frame);
}

void IrMode::backward()
{
  CommandFrame frame;
  frame.vehicleAction = CarAction::BACKWARD;
  frame.vehicleSpeed  = IR_MOTOR_SPEED;
  commandSender.send(frame);
}

void IrMode::left()
{
  CommandFrame frame;
  frame.vehicleAction = CarAction::TURN_LEFT;
  frame.vehicleSpeed  = IR_MOTOR_SPEED;
  commandSender.send(frame);
}

void IrMode::right()
{
  CommandFrame frame;
  frame.vehicleAction = CarAction::TURN_RIGHT;
  frame.vehicleSpeed  = IR_MOTOR_SPEED;
  commandSender.send(frame);
}

void IrMode::stop()
{
  CommandFrame frame;
  frame.vehicleAction = CarAction::FORCE_STOP;
  frame.vehicleSpeed  = 0;
  commandSender.send(frame);
}

void IrMode::servoCentered()
{
  CommandFrame frame;
  frame.servoHasCommand = true;
  frame.servoAngle      = IR_SERVO_CENTER;
  commandSender.send(frame);
}

void IrMode::servoLeft()
{
  CommandFrame frame;
  frame.servoHasCommand = true;
  frame.servoAngle      = IR_SERVO_LEFT_ANGLE;
  commandSender.send(frame);
}

void IrMode::servoRight()
{
  CommandFrame frame;
  frame.servoHasCommand = true;
  frame.servoAngle      = IR_SERVO_RIGHT_ANGLE;
  commandSender.send(frame);
}