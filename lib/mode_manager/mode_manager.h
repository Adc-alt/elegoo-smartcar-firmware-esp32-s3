#pragma once

// #include "../car_mode/car_mode.h"
// #include "../command_mode/car_mode.h"
#include "inputs/inputs.h"
#include "outputs/outputs.h"

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

  /** POST /command: solo se aplica en RC_MODE. */
  void onWebCommand(const char* action, int speed, unsigned long now);
  /**
   * POST /motors (diferencial): solo en BALL_FOLLOW_MODE salvo prueba aislada.
   * @param allowOutsideBallFollowMode true solo p. ej. firmware de test sin updateStates().
   */
  void onDifferential(const char* leftAction, uint8_t leftSpeed, const char* rightAction, uint8_t rightSpeed,
                      unsigned long now, OutputData& outputData, bool allowOutsideBallFollowMode = false);
  /** GET /streaming (MJPEG): permitido solo en BALL_FOLLOW_MODE. */
  bool isStreamingAllowed() const;

  // Getters
  CarMode getCurrentMode() const { return currentMode; }
  CarMode getPreviousMode() const { return previousMode; }

  // 1) Método helper para obtener la instancia persistente de IrMode
  IrMode& getIrModeInstance();
  // 2) Método helper para obtener la instancia persistente de ObstacleAvoidanceMode
  ObstacleAvoidanceMode& getObstacleAvoidanceModeInstance();
  // 3) Método helper para obtener la instancia persistente de FollowMode
  FollowMode& getFollowModeInstance();
  // 4) Método helper para obtener la instancia persistente de LineFollowingMode
  LineFollowingMode& getLineFollowingModeInstance();
  // 5) Método helper para obtener la instancia persistente de RcMode
  RcMode& getRcModeInstance();
  // 6) Método helper para obtener la instancia persistente de BallFollowMode
  BallFollowMode& getBallFollowModeInstance();

private:
  CarMode currentMode;
  CarMode previousMode;
  bool swPressedPrevious; // Estado anterior de swPressed para detectar flanco
  int modeCounter;        // Contador interno de pulsaciones (0=IDLE, 1=IR_MODE, etc.)

  static const char* ledColorForMode(CarMode mode);

  // Método para determinar que modo está activo basándose en el contador
  CarMode getModeFromCounter();

  // Método helper para obtener la instancia de Mode según CarMode (retorna nullptr para IDLE)
  Mode* getModeInstance(CarMode mode);

  /** Cambia a newMode si es distinto del actual (stop/start, sincroniza modeCounter, LED). */
  void transitionTo(CarMode newMode, OutputData& outputData);

  /** Mando IR: códigos modo 0–6 (IDLE…BALL; ver MODOS IR en ir_mode.cpp). Un pulso por pulsación (latch hasta
   * irRaw==0). */
  void trySelectModeFromIr(unsigned long irRaw, OutputData& outputData);

  static int counterForMode(CarMode mode);

  unsigned long irModeSelectLatch; // 0 = suelto; distinto de 0 = código IR ya consumido hasta soltar
};