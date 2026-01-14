// lib/mode_manager/mode_manager.cpp
#include "mode_manager.h"

#include "../ir_mode/ir_mode.h"

ModeManager::ModeManager()
    : currentMode(CarMode::IDLE), previousMode(CarMode::IDLE), swPressedPrevious(false), modeCounter(0)
{
}

IrMode& ModeManager::getIrModeInstance()
{
  // Instancia estática local (se crea solo una vez, persiste entre llamadas)
  static IrMode irModeInstance;
  return irModeInstance;
}

CarMode ModeManager::getModeFromCounter()
{
  // Convertir contador interno a modo
  // 0 = IDLE
  // 1 = IR_MODE
  // 2+ = preparado para siguiente modo (por ahora IDLE)

  if (modeCounter == 0)
  {
    return CarMode::IDLE;
  }
  else if (modeCounter == 1)
  {
    return CarMode::IR_MODE;
  }
  else
  {
    // modeCounter >= 2: por ahora volver a IDLE (aquí se agregará el siguiente modo)
    return CarMode::IDLE;
  }
}

void ModeManager::updateStates(const InputData& inputData, OutputData& outputData)
{
  // Detectar flanco de subida de swPressed (de false a true)
  bool swPressedRisingEdge = (inputData.swPressed == true && swPressedPrevious == false);

  // Si detectamos un flanco de subida, incrementar contador y cambiar de modo
  if (swPressedRisingEdge)
  {
    modeCounter++;

    // Por ahora solo tenemos 2 modos (IDLE e IR_MODE), así que resetear después de 2
    // Cuando agregues más modos, ajusta este número
    if (modeCounter >= 2)
    {
      modeCounter = 0; // Volver a IDLE después de IR_MODE
    }

    // Obtener el nuevo modo basado en el contador
    CarMode newMode = getModeFromCounter();

    // Actualizar modo
    previousMode = currentMode;
    currentMode  = newMode;

    // Serial.print("Pulsación detectada - Contador: ");
    // Serial.print(modeCounter);
    // Serial.print(" - Modo cambiado: ");
    // Serial.print(static_cast<int>(previousMode));
    // Serial.print(" -> ");
    // Serial.println(static_cast<int>(currentMode));

    // Imprimir color del LED solo cuando cambia el modo
    switch (currentMode)
    {
      case CarMode::IR_MODE:
        // Serial.println("LED Color: BLUE");
        break;
      case CarMode::IDLE:
      default:
        // Serial.println("LED Color: YELLOW");
        break;
    }
  }

  // Actualizar estado anterior para la próxima iteración
  swPressedPrevious = inputData.swPressed;

  // Asignar color del LED según el modo
  switch (currentMode)
  {
    case CarMode::IR_MODE:
      outputData.ledColor = "BLUE"; // Color para modo IR
      break;

    case CarMode::IDLE:
    default:
      outputData.ledColor = "YELLOW"; // Color para modo IDLE
      break;
  }

  // Ejecutar la lógica del modo activo
  switch (currentMode)
  {
    case CarMode::IR_MODE:
      // Usar instancia persistente del modo IR (mantiene estado entre llamadas)
      // El timeout se extiende cada vez que llega un comando IR válido
      getIrModeInstance().update(inputData, outputData);
      break;

    case CarMode::IDLE:
    default:
      // Modo IDLE: valores por defecto (parar el coche)
      outputData.action     = "free_stop";
      outputData.speed      = 0;
      outputData.servoAngle = 90;
      // ledColor ya se asignó arriba
      break;
  }
}