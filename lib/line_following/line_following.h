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
  LineState lastLineState; // Último estado de línea detectado (para LOST_*)

  // Constantes
  static constexpr int LINE_THRESHOLD = 600; // Umbral para detectar línea (valores < threshold = línea negra)
  static constexpr uint8_t SPEED      = 30;  // Velocidad del coche

  // Métodos privados
  void updateLogic(const InputData& inputData, OutputData& outputData);
  LineState determineLineState(const InputData& inputData);
};
