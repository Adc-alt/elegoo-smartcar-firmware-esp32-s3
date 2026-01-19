// lib/ir_mode/ir_mode.cpp
#include "ir_mode.h"

#include <Arduino.h>

IrMode::IrMode()
{
}
bool IrMode::update(const InputData& inputData, OutputData& outputData)
{
  unsigned long irRaw       = inputData.irRaw;
  unsigned long currentTime = millis();

  // Solo procesar si hay un comando IR válido (no cero)
  if (irRaw != 0)
  {
    processIrCommand(irRaw, outputData);
    lastCommandTime = currentTime;
    commandActive   = true;
    return true;
  }

  if (commandActive)
  {
    unsigned long elapsedTime = currentTime - lastCommandTime;

    if (elapsedTime >= COMMAND_TIMEOUT_MS)
    {
      CarActions::freeStop(outputData);
      commandActive = false;
    }
  }

  return commandActive;
}

void IrMode::processIrCommand(unsigned long irRaw, OutputData& outputData)
{
  // Mapear valores IR raw a acciones del coche
  // TODO: Reemplaza estos valores con los códigos reales de tu mando IR
  // Puedes usar Serial.println(irRaw) para ver qué valores recibes de cada botón

  // Valor para "forward" - reemplaza 3108437760 con el valor real de tu mando
  if (irRaw == 3108437760)
  {
    CarActions::forward(outputData, 80);
  }
  // Valor para "backward" - reemplaza con el valor real
  else if (irRaw == 3927310080)
  {
    CarActions::backward(outputData, 80);
  }
  // Valor para "left" - reemplaza con el valor real
  else if (irRaw == 3141861120)
  {
    CarActions::turnLeft(outputData, 80);
  }
  // Valor para "right" - reemplaza con el valor real
  else if (irRaw == 3158572800)
  {
    CarActions::turnRight(outputData, 80);
  }
  // Valor para "stop" - reemplaza con el valor real
  else if (irRaw == 3208707840)
  {
    CarActions::forceStop(outputData);
  }
  else if (irRaw == 4061003520)
  {
    CarActions::setServoAngle(outputData, 20);
  }
  else if (irRaw == 3910598400)
  {
    CarActions::setServoAngle(outputData, 160);
  }
  else if (irRaw == 3860463360)
  {
    CarActions::setServoAngle(outputData, 90);
  }
  else
  {
    // Comando desconocido - puedes agregar Serial.println(irRaw) aquí para debuggear
    CarActions::freeStop(outputData);
  }
}

String statusToString(IR_STATUS status)
{
  switch (status)
  {
    case IR_IDLE:
      return "IR_IDLE";
    case IR_FORWARD:
      return "IR_FORWARD";
    case IR_BACKWARD:
      return "IR_BACKWARD";
    case IR_LEFT:
      return "IR_LEFT";
    case IR_RIGHT:
      return "IR_RIGHT";
    case IR_STOP:
      return "IR_STOP";
    case IR_SERVO_LEFT:
      return "IR_SERVO_LEFT";
    case IR_SERVO_RIGHT:
      return "IR_SERVO_RIGHT";
    case IR_SERVO_CENTER:
      return "IR_SERVO_CENTER";
    default:
      return "IR_UNKNOWN";
  }
}