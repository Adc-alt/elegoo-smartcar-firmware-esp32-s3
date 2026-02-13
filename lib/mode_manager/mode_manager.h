#pragma once

// #include "../car_mode/car_mode.h"
// #include "../command_mode/car_mode.h"
#include "../inputs/inputs.h"
#include "../outputs/outputs.h"

#include <Arduino.h>

// Enum modos
enum class CarMode
{
  IR_MODE,                 // Modo control por IR
  OBSTACLE_AVOIDANCE_MODE, // Modo de evitar obstáculos
  FOLLOW_MODE,
  LINE_FOLLOWING_MODE,
  RC_MODE,
  BALL_FOLLOW_MODE,
  IDLE // Inactivo
};

class Mode
{
public:
  virtual ~Mode() = default;
  // Métodos para iniciar y detener el modo
  virtual void startMode() {}
  virtual void stopMode(OutputData& outputData) {}
  // Actualiza el estado del modo basándose en inputData
  // Modifica outputData según la lógica del Modo
  // Retorna true si el modo está activo, false si no
  virtual bool update(const InputData& inputData, OutputData& outputData) = 0;
  // virtual const char* getName() const                                     = 0;
};

// Forward declaration para evitar dependencia circular
class IrMode;
class ObstacleAvoidanceMode;
class FollowMode;
class LineFollowingMode;
class RcMode;
class BallFollowMode;
class ModeManager
{
public:
  ModeManager();

  // Actualiza estados según el modo activo
  // Recibe receiveJson (entrada) y modifica outputData
  void updateStates(const InputData& inputData, OutputData& outputData);
  // Getters
  CarMode getCurrentMode() const { return currentMode; }
  CarMode getPreviousMode() const { return previousMode; }
  RcMode& getRcModeInstance();
  
  
private:
  CarMode currentMode;
  CarMode previousMode;
  bool swPressedPrevious; // Estado anterior de swPressed para detectar flanco
  int modeCounter;        // Contador interno de pulsaciones (0=IDLE, 1=IR_MODE, etc.)

  static const char* ledColorForMode(CarMode mode);

  // Método para determinar que modo está activo basándose en el contador
  CarMode getModeFromCounter();

  // Método helper para obtener la instancia persistente de IrMode
  IrMode& getIrModeInstance();

  // Método helper para obtener la instancia persistente de ObstacleAvoidanceMode
  // Método helper para obtener la instancia persistente de ObstacleAvoidanceMode
  ObstacleAvoidanceMode& getObstacleAvoidanceModeInstance();

  // Método helper para obtener la instancia persistente de FollowMode
  FollowMode& getFollowModeInstance();

  // Método helper para obtener la instancia persistente de LineFollowingMode
  LineFollowingMode& getLineFollowingModeInstance();

  // Método helper para obtener la instancia persistente de RcMode

  // Método helper para obtener la instancia persistente de BallFollowMode
  BallFollowMode& getBallFollowModeInstance();

  // Método helper para obtener la instancia de Mode según CarMode (retorna nullptr para IDLE)
  Mode* getModeInstance(CarMode mode);
};