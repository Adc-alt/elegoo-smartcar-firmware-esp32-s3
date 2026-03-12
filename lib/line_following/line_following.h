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
  LineState lastLineState; // Último estado de línea detectado (para LOST_*)

  // Constantes mínimas
  static constexpr int LINE_THRESHOLD   = 650; // Umbral para detectar línea (valores > threshold = línea negra)
  static constexpr uint8_t SPEED       = 5;   // Velocidad al corregir (giros y LOST)
  static constexpr uint8_t SPEED_STRAIGHT = 20; // Velocidad en recta (CENTER)

  // Métodos privados
  void updateLogic(const InputData& inputData, OutputData& outputData);
  LineState determineLineState(const InputData& inputData);
};