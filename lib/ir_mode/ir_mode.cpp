// lib/ir_mode/ir_mode.cpp
#include "ir_mode.h"

#include "../inputs/inputs.h"

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
      outputData.action = "free_stop";
      outputData.speed  = 0;
      commandActive     = false;
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
    outputData.action = "forward";
    outputData.speed  = 80;
  }
  // Valor para "backward" - reemplaza con el valor real
  else if (irRaw == 3927310080)
  {
    outputData.action = "backward";
    outputData.speed  = 80;
  }
  // Valor para "left" - reemplaza con el valor real
  else if (irRaw == 3141861120)
  {
    outputData.action = "turn_left";
    outputData.speed  = 80;
  }
  // Valor para "right" - reemplaza con el valor real
  else if (irRaw == 3158572800)
  {
    outputData.action = "turn_right";
    outputData.speed  = 80;
  }
  // Valor para "stop" - reemplaza con el valor real
  else if (irRaw == 3208707840)
  {
    outputData.action = "force_stop";
    outputData.speed  = 0;
  }
  else if (irRaw == 4061003520)
  {
    outputData.servoAngle = 160;
  }
  else if (irRaw == 3910598400)
  {
    outputData.servoAngle = 20;
  }
  else if (irRaw == 3860463360)
  {
    outputData.servoAngle = 90;
  }
  else
  {
    // Comando desconocido - puedes agregar Serial.println(irRaw) aquí para debuggear
    outputData.action = "free_stop";
    outputData.speed  = 0;
  }
}