// lib/mode_manager/mode_manager.cpp
//
// Orden por flujo de ejecución (típico: un frame desde updateStates):
//   1) Helpers anónimos (IR → CarMode, longitud del ciclo del switch)
//   2) Constructor
//   3) updateStates — orquestador del frame
//   4) trySelectModeFromIr — bloque 0 dentro de updateStates (mando IR solo selección de modo)
//   5) getModeFromCounter / counterForMode — bloque 1 (switch físico → siguiente modo)
//   6) Singletons por modo + getModeInstance — usados al cambiar de modo y en el switch de lógica activa
//   7) ledColorForMode — usado al entrar en un modo
//   8) transitionTo — transición común (IR o switch)
//   9) Entradas paralelas al loop: web RC, diferencial ball-follow, streaming
#include "mode_manager.h"

#include "../car_actions/car_actions.h"
#include "../modes/ball_follow_mode/ball_follow_mode.h"
#include "../modes/follow_mode/follow_mode.h"
#include "../modes/ir_mode/ir_mode.h"
#include "../modes/line_following/line_following.h"
#include "../modes/obstacle_avoidance/obstacle_avoidance.h"
#include "../modes/rc_mode/rc_mode.h"
#include "ir_remote_codes.h"

namespace
{
constexpr int kModeCycleLength = 7; // contador 0..6: IDLE … BALL_FOLLOW

bool mapIrRawToCarMode(unsigned long irRaw, CarMode& out)
{
  using namespace IrRemote;
  switch (irRaw)
  {
    case kSelectIdle:
      out = CarMode::IDLE;
      return true;
    case kSelectIrMode:
      out = CarMode::IR_MODE;
      return true;
    case kSelectObstacle:
      out = CarMode::OBSTACLE_AVOIDANCE_MODE;
      return true;
    case kSelectFollow:
      out = CarMode::FOLLOW_MODE;
      return true;
    case kSelectLine:
      out = CarMode::LINE_FOLLOWING_MODE;
      return true;
    case kSelectRc:
      out = CarMode::RC_MODE;
      return true;
    case kSelectBallFollow:
      out = CarMode::BALL_FOLLOW_MODE;
      return true;
    default:
      return false;
  }
}
} // namespace

// --- Arranque ---

ModeManager::ModeManager()
  : currentMode(CarMode::IDLE)
  , previousMode(CarMode::IDLE)
  , swPressedPrevious(false)
  , modeCounter(0)
  , irModeSelectLatch(0)
{
  // Serial.println("ModeManager: Inicializado - Modo IDLE");
}

// --- Un frame: updateStates → (IR latch / switch) → lógica del modo activo ---

void ModeManager::updateStates(const InputData& inputData, OutputData& outputData)
{
  //**************************** 0) MANDO IR: SOLO SELECCIÓN DE MODO ****************************//
  // No mueve coche ni servo; IrMode solo ve códigos de conducción. Latch hasta irRaw==0 (suelta tecla).
  if (inputData.irRaw == 0) // EVENTO 1: MANDO IR
    irModeSelectLatch = 0;
  else
    trySelectModeFromIr(inputData.irRaw, outputData);

  //**************************** 1) CAMBIO DE MODO ****************************//

  // Detectar flanco de subida de swPressed (de false a true)
  bool swPressedRisingEdge = (inputData.swPressed == true && swPressedPrevious == false); // EVENTO 2: BOTON

  // Si detectamos un flanco de subida, incrementar contador y cambiar de modo
  if (swPressedRisingEdge)
  {
    // 7 modos: IDLE, IR_MODE, OBSTACLE_AVOIDANCE_MODE, FOLLOW_MODE, LINE_FOLLOWING_MODE, RC_MODE, BALL_FOLLOW_MODE
    modeCounter = (modeCounter + 1) % kModeCycleLength;

    // Obtener el nuevo modo basado en el contador
    CarMode newMode = getModeFromCounter();

    outputData.modeOrdinal = modeCounter;
    transitionTo(newMode, outputData);
  }

  // Actualizar estado anterior para la próxima iteración (siempre, no solo en flanco)
  swPressedPrevious = inputData.swPressed;

  //**************************** 2) LÓGICA MODO ACTIVO ****************************//
  // Ejecutar la lógica del modo activo (siempre, no solo cuando se presiona el botón)
  switch (currentMode)
  {
    case CarMode::IR_MODE:
      // Usar instancia persistente del modo IR (mantiene estado entre llamadas)
      // El timeout se extiende cada vez que llega un comando IR válido
      getIrModeInstance().update(inputData, outputData);
      break;

    case CarMode::OBSTACLE_AVOIDANCE_MODE:
      // Usar instancia persistente del modo OBSTACLE_AVOIDANCE_MODE (mantiene estado entre llamadas)
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

void ModeManager::trySelectModeFromIr(unsigned long irRaw, OutputData& outputData)
{
  CarMode target;
  if (!mapIrRawToCarMode(irRaw, target))
    return;

  if (irModeSelectLatch == irRaw) // Antirebote: mismo código hasta soltar tecla
    return;

  irModeSelectLatch = irRaw;
  transitionTo(target, outputData);
}

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

// Instancias persistentes (static local en cada getter; orden = counterForMode 1..6).
// Mantienen estado entre llamadas al loop (timeouts, últimos comandos, etc.).

IrMode& ModeManager::getIrModeInstance()
{
  static IrMode irModeInstance;
  return irModeInstance;
}

ObstacleAvoidanceMode& ModeManager::getObstacleAvoidanceModeInstance()
{
  static ObstacleAvoidanceMode obstacleAvoidanceModeInstance;
  return obstacleAvoidanceModeInstance;
}

FollowMode& ModeManager::getFollowModeInstance()
{
  static FollowMode followModeInstance;
  return followModeInstance;
}

LineFollowingMode& ModeManager::getLineFollowingModeInstance()
{
  static LineFollowingMode lineFollowingModeInstance;
  return lineFollowingModeInstance;
}

RcMode& ModeManager::getRcModeInstance()
{
  static RcMode rcModeInstance;
  return rcModeInstance;
}

BallFollowMode& ModeManager::getBallFollowModeInstance()
{
  static BallFollowMode ballFollowModeInstance;
  return ballFollowModeInstance;
}

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
      return "YELLOW";
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
  // Alinear contador con el modo actual para que el siguiente pulso del switch no desincronice ciclo físico y modo.
  modeCounter = counterForMode(newMode);

  outputData.modeOrdinal = static_cast<uint8_t>(static_cast<int>(newMode));

  Mode* newModeInstance = getModeInstance(currentMode);
  if (newModeInstance != nullptr)
    newModeInstance->startMode();

  CarActions::setLedColor(outputData, ledColorForMode(currentMode));
}

// --- Callbacks / HTTP (fuera del orden lineal de updateStates) ---

void ModeManager::onWebCommand(const char* action, int speed, unsigned long now)
{
  if (currentMode != CarMode::RC_MODE)
    return;
  getRcModeInstance().onWebCommandReceived(action, speed, now);
}

void ModeManager::onDifferential(const char* leftAction, uint8_t leftSpeed, const char* rightAction, uint8_t rightSpeed,
                                 unsigned long now, OutputData& outputData, bool allowOutsideBallFollowMode)
{
  if (!allowOutsideBallFollowMode && currentMode != CarMode::BALL_FOLLOW_MODE)
    return;

  outputData.leftAction  = leftAction ? leftAction : "forward";
  outputData.leftSpeed   = leftSpeed;
  outputData.rightAction = rightAction ? rightAction : "forward";
  outputData.rightSpeed  = rightSpeed;
  getBallFollowModeInstance().onDifferentialReceived(now);
}

bool ModeManager::isStreamingAllowed() const
{
  return currentMode == CarMode::BALL_FOLLOW_MODE;
}
