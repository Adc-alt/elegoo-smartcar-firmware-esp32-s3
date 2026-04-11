#pragma once

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

// Orden API (alto nivel): updateStates por frame → instancias de modo (mismo orden que el switch en updateStates)
// → callbacks web/stream. Ver mode_manager.cpp para el orden detallado de implementación.
class ModeManager
{
public:
  ModeManager();

  void updateStates(const InputData& inputData, OutputData& outputData);

  CarMode getCurrentMode() const { return currentMode; }
  CarMode getPreviousMode() const { return previousMode; }

  IrMode& getIrModeInstance();
  ObstacleAvoidanceMode& getObstacleAvoidanceModeInstance();
  FollowMode& getFollowModeInstance();
  LineFollowingMode& getLineFollowingModeInstance();
  RcMode& getRcModeInstance();
  BallFollowMode& getBallFollowModeInstance();

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

private:
  CarMode currentMode;
  CarMode previousMode;
  bool swPressedPrevious;          // Estado anterior de swPressed para detectar flanco
  int modeCounter;                 // Contador interno de pulsaciones (0=IDLE, 1=IR_MODE, etc.)
  unsigned long irModeSelectLatch; // 0 = suelto; distinto de 0 = código IR ya consumido hasta soltar

  /** Mando IR: selección de CarMode. Latch hasta irRaw==0 (ir_remote_codes.h kSelect*). */
  void trySelectModeFromIr(unsigned long irRaw, OutputData& outputData);

  CarMode getModeFromCounter();
  static int counterForMode(CarMode mode);

  static const char* ledColorForMode(CarMode mode);
  Mode* getModeInstance(CarMode mode);

  /** Cambia a newMode si es distinto del actual (stop/start, sincroniza modeCounter, LED). */
  void transitionTo(CarMode newMode, OutputData& outputData);
};