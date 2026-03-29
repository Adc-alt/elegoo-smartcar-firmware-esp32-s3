// lib/mode_manager/mode_manager.cpp
#include "mode_manager.h"

#include "../ball_follow_mode/ball_follow_mode.h"
#include "../car_actions/car_actions.h"
#include "../follow_mode/follow_mode.h"
#include "../ir_mode/ir_mode.h"
#include "../line_following/line_following.h"
#include "../obstacle_avoidance/obstacle_avoidance.h"
#include "../rc_mode/rc_mode.h"

ModeManager::ModeManager()
  : currentMode(CarMode::IDLE)
  , previousMode(CarMode::IDLE)
  , swPressedPrevious(false)
  , modeCounter(0)
  , irModeSelectLatch(0)
{
  // Serial.println("ModeManager: Inicializado - Modo IDLE");
}

namespace {

// Mando IR → modo (0=IDLE … 6=BALL; mismos códigos que MODOS IR en lib/ir_mode/ir_mode.cpp)
bool mapIrRawToCarMode(unsigned long irRaw, CarMode& out)
{
  switch (irRaw)
  {
    case 2907897600UL:
      out = CarMode::IDLE;
      return true;
    case 3910598400UL:
      out = CarMode::IR_MODE;
      return true;
    case 3860463360UL:
      out = CarMode::OBSTACLE_AVOIDANCE_MODE;
      return true;
    case 4061003520UL:
      out = CarMode::FOLLOW_MODE;
      return true;
    case 4077715200UL:
      out = CarMode::LINE_FOLLOWING_MODE;
      return true;
    case 3877175040UL:
      out = CarMode::RC_MODE;
      return true;
    case 2707357440UL:
      out = CarMode::BALL_FOLLOW_MODE;
      return true;
    default:
      return false;
  }
}

} // namespace

int ModeManager::counterForMode(CarMode mode)
{
  switch (mode)
  {
    case CarMode::IDLE:
      return 0;
    case CarMode::IR_MODE:
      return 1;
    case CarMode::OBSTACLE_AVOIDANCE_MODE:
      return 2;
    case CarMode::FOLLOW_MODE:
      return 3;
    case CarMode::LINE_FOLLOWING_MODE:
      return 4;
    case CarMode::RC_MODE:
      return 5;
    case CarMode::BALL_FOLLOW_MODE:
      return 6;
    default:
      return 0;
  }
}

void ModeManager::transitionTo(CarMode newMode, OutputData& outputData)
{
  if (newMode == currentMode)
    return;

  Mode* previousModeInstance = getModeInstance(currentMode);
  if (previousModeInstance != nullptr)
    previousModeInstance->stopMode(outputData);

  previousMode = currentMode;
  currentMode  = newMode;
  modeCounter  = counterForMode(newMode);

  Mode* newModeInstance = getModeInstance(currentMode);
  if (newModeInstance != nullptr)
    newModeInstance->startMode();

  CarActions::setLedColor(outputData, ledColorForMode(currentMode));
}

void ModeManager::trySelectModeFromIr(unsigned long irRaw, OutputData& outputData)
{
  CarMode target;
  if (!mapIrRawToCarMode(irRaw, target))
    return;

  if (irModeSelectLatch == irRaw)
    return;

  irModeSelectLatch = irRaw;
  transitionTo(target, outputData);
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
    case CarMode::FOLLOW_MODE:
      return "FOLLOW_MODE";
    case CarMode::LINE_FOLLOWING_MODE:
      return "LINE_FOLLOWING_MODE";
    case CarMode::RC_MODE:
      return "RC_MODE";
    case CarMode::BALL_FOLLOW_MODE:
      return "BALL_FOLLOW_MODE";
    case CarMode::IDLE:
      return "IDLE";
    default:
      return "UNKNOWN";
  }
}

void ModeManager::updateStates(const InputData& inputData, OutputData& outputData)
{
  //**************************** 0) MANDO IR: SOLO SELECCIÓN DE MODO ****************************//
  // No mueve coche ni servo; IrMode solo ve códigos de conducción. Latch hasta irRaw==0 (suelta tecla).
  if (inputData.irRaw == 0)
    irModeSelectLatch = 0;
  else
    trySelectModeFromIr(inputData.irRaw, outputData);

  //**************************** 1) CAMBIO DE MODO ****************************//

  // Detectar flanco de subida de swPressed (de false a true)
  bool swPressedRisingEdge = (inputData.swPressed == true && swPressedPrevious == false);

  // Si detectamos un flanco de subida, incrementar contador y cambiar de modo
  if (swPressedRisingEdge)
  {
    // 7 modos: IDLE, IR_MODE, OBSTACLE_AVOIDANCE_MODE, FOLLOW_MODE, LINE_FOLLOWING_MODE, RC_MODE, BALL_FOLLOW_MODE
    modeCounter = (modeCounter + 1) % 7;

    // Obtener el nuevo modo basado en el contador
    CarMode newMode = getModeFromCounter();

    transitionTo(newMode, outputData);

    //**************************** 2) LED SEGUN MODO ****************************//
    // Mismo comportamiento que antes: refrescar LED en cada pulsación del switch físico
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
      getObstacleAvoidanceModeInstance().update(inputData, outputData);
      break;
    case CarMode::FOLLOW_MODE:
      // Usar instancia persistente del modo FOLLOW_MODE (mantiene estado entre llamadas)
      getFollowModeInstance().update(inputData, outputData);
      break;
    case CarMode::LINE_FOLLOWING_MODE:
      // Usar instancia persistente del modo LINE_FOLLOWING_MODE (mantiene estado entre llamadas)
      getLineFollowingModeInstance().update(inputData, outputData);
      break;

    case CarMode::RC_MODE:
      // Modo control remoto por web/WiFi (comandos vienen del callback de webHost)
      getRcModeInstance().update(inputData, outputData);
      break;

    case CarMode::BALL_FOLLOW_MODE:
      // Modo seguimiento de bola verde (visión)
      getBallFollowModeInstance().update(inputData, outputData);
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
    case CarMode::FOLLOW_MODE:
      return "PURPLE";
    case CarMode::LINE_FOLLOWING_MODE:
      return "WHITE";
    case CarMode::RC_MODE:
      return "SALMON";
    case CarMode::BALL_FOLLOW_MODE:
      return "CYAN";
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
    case 3:
      return CarMode::FOLLOW_MODE;
    case 4:
      return CarMode::LINE_FOLLOWING_MODE;
    case 5:
      return CarMode::RC_MODE;
    case 6:
      return CarMode::BALL_FOLLOW_MODE;
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
ObstacleAvoidanceMode& ModeManager::getObstacleAvoidanceModeInstance()
{
  // Instancia estática local (se crea solo una vez, persiste entre llamadas)
  static ObstacleAvoidanceMode obstacleAvoidanceModeInstance;
  return obstacleAvoidanceModeInstance;
}

// Getter para obtener la instancia persistente de FollowMode
FollowMode& ModeManager::getFollowModeInstance()
{
  // Instancia estática local (se crea solo una vez, persiste entre llamadas)
  static FollowMode followModeInstance;
  return followModeInstance;
}

// Getter para obtener la instancia persistente de LineFollowingMode
LineFollowingMode& ModeManager::getLineFollowingModeInstance()
{
  // Instancia estática local (se crea solo una vez, persiste entre llamadas)
  static LineFollowingMode lineFollowingModeInstance;
  return lineFollowingModeInstance;
}

// Getter para obtener la instancia persistente de RcMode
RcMode& ModeManager::getRcModeInstance()
{
  // Instancia estática local (se crea solo una vez, persiste entre llamadas)
  static RcMode rcModeInstance;
  return rcModeInstance;
}

// Getter para obtener la instancia persistente de BallFollowMode
BallFollowMode& ModeManager::getBallFollowModeInstance()
{
  // Instancia estática local (se crea solo una vez, persiste entre llamadas)
  static BallFollowMode ballFollowModeInstance;
  return ballFollowModeInstance;
}
// Helper para obtener la instancia de Mode según CarMode (retorna nullptr para IDLE)
Mode* ModeManager::getModeInstance(CarMode mode)
{
  switch (mode)
  {
    case CarMode::IR_MODE:
      return &getIrModeInstance();
    case CarMode::OBSTACLE_AVOIDANCE_MODE:
      return &getObstacleAvoidanceModeInstance();
    case CarMode::FOLLOW_MODE:
      return &getFollowModeInstance();
    case CarMode::LINE_FOLLOWING_MODE:
      return &getLineFollowingModeInstance();
    case CarMode::RC_MODE:
      return &getRcModeInstance();
    case CarMode::BALL_FOLLOW_MODE:
      return &getBallFollowModeInstance();
    case CarMode::IDLE:
    default:
      return nullptr; // IDLE no tiene instancia de Mode
  }
}
