// lib/mode_manager/mode_manager.cpp
#include "mode_manager.h"

#include "../car_actions/car_actions.h"
#include "../ir_mode/ir_mode.h"
#include "../obstacle_avoidance/obstacle_avoidance.h"

ModeManager::ModeManager()
  : currentMode(CarMode::IDLE)
  , previousMode(CarMode::IDLE)
  , swPressedPrevious(false)
  , modeCounter(0)
{
  Serial.println("ModeManager: Inicializado - Modo IDLE");
}

// Helper para convertir CarMode a string
const char* modeToString(CarMode mode)
{
  switch (mode)
  {
    case CarMode::IR_MODE:
      return "IR_MODE";
    case CarMode::OBSTACLE_AVOIDANCE_MODE:
      return "OBSTACLE_AVOIDANCE_MODE";
    case CarMode::IDLE:
      return "IDLE";
    default:
      return "UNKNOWN";
  }
}

void ModeManager::updateStates(const InputData& inputData, OutputData& outputData)
{
  //**************************** 1) CAMBIO DE MODO ****************************//

  // Detectar flanco de subida de swPressed (de false a true)
  bool swPressedRisingEdge = (inputData.swPressed == true && swPressedPrevious == false);

  // Si detectamos un flanco de subida, incrementar contador y cambiar de modo
  if (swPressedRisingEdge)
  {
    // Por ahora solo tenemos 2 modos (IDLE e IR_MODE), así que resetear después de 2
    modeCounter = (modeCounter + 1) % 3;

    // Obtener el nuevo modo basado en el contador
    CarMode newMode = getModeFromCounter();

    // Actualizar modo
    if (newMode != currentMode)
    {
      // Detener el modo anterior antes de cambiar (usar currentMode antes de actualizarlo)
      Mode* previousModeInstance = getModeInstance(currentMode);
      if (previousModeInstance != nullptr)
      {
        previousModeInstance->stopMode(outputData);
      }

      // Actualizar variables de modo
      previousMode = currentMode;
      currentMode  = newMode;

      // Iniciar el nuevo modo después de cambiar
      Mode* newModeInstance = getModeInstance(currentMode);
      if (newModeInstance != nullptr)
      {
        newModeInstance->startMode();
      }

      // Print del cambio de modo
      Serial.print("ModeManager: ");
      Serial.print(modeToString(previousMode));
      Serial.print(" -> ");
      Serial.print(modeToString(currentMode));
      Serial.print(" (Contador: ");
      Serial.print(modeCounter);
      Serial.println(")");
    }

    //**************************** 2) LED SEGUN MODO ****************************//
    // Imprimir color del LED solo cuando cambia el modo
    CarActions::setLedColor(outputData, ModeManager::ledColorForMode(currentMode));
  }

  // Actualizar estado anterior para la próxima iteración (siempre, no solo en flanco)
  swPressedPrevious = inputData.swPressed;

  //**************************** 3) LÓGICA MODO ACTIVO ****************************//
  // Ejecutar la lógica del modo activo (siempre, no solo cuando se presiona el botón)
  switch (currentMode)
  {
    case CarMode::IR_MODE:
      // Usar instancia persistente del modo IR (mantiene estado entre llamadas)
      // El timeout se extiende cada vez que llega un comando IR válido
      getIrModeInstance().update(inputData, outputData);
      break;

    case CarMode::OBSTACLE_AVOIDANCE_MODE:
      // Usar instancia persistente del modo OBTABLE_AVOIDANCE_MODE (mantiene estado entre llamadas)
      // El timeout se extiende cada vez que llega un comando IR válido
      getObtableAvoidanceModeInstance().update(inputData, outputData);
      break;

    case CarMode::IDLE:
    default:
      // Modo IDLE: valores por defecto (parar el coche)
      CarActions::freeStop(outputData);
      CarActions::setServoAngle(outputData, 90);
      break;
  }
}

// Helper para obtener el color del LED basado en el modo
const char* ModeManager::ledColorForMode(CarMode mode)
{
  switch (mode)
  {
    case CarMode::IR_MODE:
      return "BLUE";
    case CarMode::OBSTACLE_AVOIDANCE_MODE:
      return "GREEN";
    case CarMode::IDLE:
      return "YELLOW";
    default:
      return "YELLOW"; // Por defecto, modo IDLE
  }
}

// Helper para obtener el modo basado en el contador
CarMode ModeManager::getModeFromCounter()
{
  switch (modeCounter)
  {
    case 0:
      return CarMode::IDLE;
    case 1:
      return CarMode::IR_MODE;
    case 2:
      return CarMode::OBSTACLE_AVOIDANCE_MODE;
    default:
      return CarMode::IDLE;
  }
}

// Getter para obtener la instancia persistente de IrMode
IrMode& ModeManager::getIrModeInstance()
{
  // Instancia estática local (se crea solo una vez, persiste entre llamadas)
  static IrMode irModeInstance;
  return irModeInstance;
}

// Getter para obtener la instancia persistente de ObstacleAvoidanceMode
ObstacleAvoidanceMode& ModeManager::getObtableAvoidanceModeInstance()
{
  // Instancia estática local (se crea solo una vez, persiste entre llamadas)
  static ObstacleAvoidanceMode obstacleAvoidanceModeInstance;
  return obstacleAvoidanceModeInstance;
}

// Helper para obtener la instancia de Mode según CarMode (retorna nullptr para IDLE)
Mode* ModeManager::getModeInstance(CarMode mode)
{
  switch (mode)
  {
    case CarMode::IR_MODE:
      return &getIrModeInstance();
    case CarMode::OBSTACLE_AVOIDANCE_MODE:
      return &getObtableAvoidanceModeInstance();
    case CarMode::IDLE:
    default:
      return nullptr; // IDLE no tiene instancia de Mode
  }
}
