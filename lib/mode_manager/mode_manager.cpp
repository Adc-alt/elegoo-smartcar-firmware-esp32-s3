// lib/mode_manager/mode_manager.cpp
#include "mode_manager.h"

#include "../car_actions/car_actions.h"
#include "../ir_mode/ir_mode.h"
#include "../obstacle_avoidance/obstacle_avoidance.h"

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

ObstacleAvoidanceMode& ModeManager::getObtableAvoidanceModeInstance()
{
  // Instancia estática local (se crea solo una vez, persiste entre llamadas)
  static ObstacleAvoidanceMode obstacleAvoidanceModeInstance;
  return obstacleAvoidanceModeInstance;
}

CarMode ModeManager::getModeFromCounter()
{
  // Convertir contador interno a modo
  // 0 = IDLE
  // 1 = IR_MODE
  // 2 = OBTABLE_AVOIDANCE_MODE

  if (modeCounter == 0)
  {
    return CarMode::IDLE;
  }
  else if (modeCounter == 1)
  {
    return CarMode::IR_MODE;
  }
  else if (modeCounter == 2)
  {
    return CarMode::OBTABLE_AVOIDANCE_MODE;
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
    if (modeCounter >= 3)
    {
      modeCounter = 0; // Volver a IDLE después de OBTABLE_AVOIDANCE_MODE
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
      case CarMode::OBTABLE_AVOIDANCE_MODE:
        // Serial.println("LED Color: GREEN");
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
      CarActions::setLedColor(outputData, "BLUE"); // Color para modo IR
      break;

    case CarMode::OBTABLE_AVOIDANCE_MODE:
      CarActions::setLedColor(outputData, "GREEN"); // Color para modo OBTABLE_AVOIDANCE_MODE
      break;

    case CarMode::IDLE:
    default:
      CarActions::setLedColor(outputData, "YELLOW"); // Color para modo IDLE
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

    case CarMode::OBTABLE_AVOIDANCE_MODE:
      // Usar instancia persistente del modo OBTABLE_AVOIDANCE_MODE (mantiene estado entre llamadas)
      // El timeout se extiende cada vez que llega un comando IR válido
      getObtableAvoidanceModeInstance().update(inputData, outputData);
      break;

    case CarMode::IDLE:
    default:
      // Modo IDLE: valores por defecto (parar el coche)
      CarActions::freeStop(outputData);
      CarActions::setServoAngle(outputData, 90);
      CarActions::setLedColor(outputData, "YELLOW");
      break;
  }
}