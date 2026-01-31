#pragma once

#include "../car_actions/car_actions.h"
#include "../inputs/inputs.h"
#include "../mode_manager/mode_manager.h"
#include "../outputs/outputs.h"

enum class LineFollowingModeState
{
  IDLE,
  MOVING_FORWARD,
  SMOOTH_TURNING_LEFT,
  SMOOTH_TURNING_RIGHT,
  TURNING_LEFT,
  TURNING_RIGHT,
};

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

class LineFollowingMode : public Mode
{
public:
  LineFollowingMode();

  // Métodos para iniciar y detener en modo
  void startMode() override;
  void stopMode(OutputData& outputData) override;

  // Actualiza el estado del modo
  bool update(const InputData& inputData, OutputData& outputData) override;

private:
  // Estados del modo
  LineFollowingModeState currentState;
  LineFollowingModeState previousState; // Estado anterior para detectar cambios // e imprimir los cambios de estado
  LineState lastLineState;              // Último estado de línea detectado (para LOST_*)

  // Control temporal de pulsos de corrección
  unsigned long turnPulseStartTime; // Tiempo de inicio del pulso de giro
  bool isTurnPulseActive;           // Flag para saber si hay un pulso activo
  bool isLostRecoveryPulse;         // True si el pulso es por pérdida (0 0 0) con memoria
  bool lostRecoveryConsumed;        // Evita repetir pulsos mientras siga en 0 0 0

  // [desactivado] Estabilización tras giro: pulso corto solo forward para no reaccionar al overshoot
  // unsigned long forwardStabilizationStartTime;
  // bool isForwardStabilizationActive;

  // Constantes
  static constexpr int LINE_THRESHOLD       = 650; // Umbral para detectar línea (valores > threshold = línea negra)
  static constexpr uint8_t SPEED            = 20;  // Velocidad del coche
  static constexpr uint8_t SPEED_CORRECTION = 50;  // Velocidad de corrección
  static constexpr uint8_t SPEED_CORRECTION_STRONG = 50; // Velocidad de corrección
  static constexpr unsigned long SMOOTH_TURN_PULSE_MS =
    15; // Duración del pulso para giros suaves (ms) – más corto para menos overshoot
  static constexpr unsigned long STRONG_TURN_PULSE_MS =
    70; // Duración del pulso para giros fuertes (ms) – más corto para menos overshoot
  // [desactivado] static constexpr unsigned long FORWARD_STABILIZATION_MS =
  //   1; // Tras cada giro: ms solo forward antes de aceptar nueva corrección

  // Métodos privados
  void updateLogic(const InputData& inputData, OutputData& outputData);
  LineState determineLineState(const InputData& inputData);
};
