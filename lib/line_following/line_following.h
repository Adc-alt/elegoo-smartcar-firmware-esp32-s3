#pragma once

#include "../car_actions/car_actions.h"
#include "../inputs/inputs.h"
#include "../mode_manager/mode_manager.h"
#include "../outputs/outputs.h"

enum class LineState
{
  CENTER,       // 010 -> line detected in the center
  CENTER_LEFT,  // 100 -> line detected on the left
  CENTER_RIGHT, // 001 -> line detected on the right
  LEFT,         // 110 -> line detected on left + center
  RIGHT,        // 011 -> line detected on center + right
  LOST_LEFT,    // 000 -> line lost, last seen on the left
  LOST_RIGHT,   // 000 -> line lost, last seen on the right
  LOST,
};

// Observable state: what the line-following mode is doing this frame
enum class LineFollowingModeState
{
  ON_LINE,          // CENTER -> going straight
  LOST,             // No line -> stopped
  RECOVERING_LEFT,  // LOST_LEFT -> turning left continuously
  RECOVERING_RIGHT, // LOST_RIGHT -> turning right continuously
  CORRECTING_LEFT,  // LEFT/CENTER_LEFT -> pulse turn or forward between pulses
  CORRECTING_RIGHT, // RIGHT/CENTER_RIGHT -> pulse turn or forward between pulses
};

class LineFollowingMode : public Mode
{
public:
  LineFollowingMode();

  // Métodos para iniciar y detener en modo
  void startMode() override;
  void stopMode(OutputData& outputData) override;

  // Actualiza el estado del modo
  bool update(const InputData& inputData, OutputData& outputData) override;

  // Observabilidad: estado actual del modo (qué está haciendo el coche en este frame)
  LineFollowingModeState getModeState() const { return currentModeState; }

private:
  LineState lastLineState; // Último estado de línea detectado (para LOST_*)
  LineFollowingModeState currentModeState;

  // Pulso de giro (turnLeft/turnRight) durante PULSE_MS; entre pulsos se avanza recto
  bool isPulseActive;
  unsigned long pulseStartTime;
  bool correctionPulseDone; // true = ya hicimos el pulso de giro de este ciclo de corrección

  // Constantes mínimas
  static constexpr int LINE_THRESHOLD     = 650; // Umbral para detectar línea (valores > threshold = línea negra)
  static constexpr uint8_t SPEED          = 15;  // Velocidad al corregir (giros y LOST)
  static constexpr uint8_t SPEED_STRAIGHT = 25;  // Velocidad en recta (CENTER)
  static constexpr unsigned long PULSE_MS = 1;   // ms (entero: 1, 2, 3, 5...; 0.5 trunca a 0)

  // Métodos privados
  void updateLogic(const InputData& inputData, OutputData& outputData);
  LineState determineLineState(const InputData& inputData);
};