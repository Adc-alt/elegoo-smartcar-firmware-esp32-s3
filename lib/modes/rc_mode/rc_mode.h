#pragma once

#include "../car_actions/car_actions.h"
#include "inputs/inputs.h"
#include "../mode_manager/mode_manager.h"
#include "outputs/outputs.h"

/**
 * Modo de control remoto por web/WiFi (equivalente a IR_MODE pero desde móvil/PC).
 * Recibe comandos a través de POST /command. El estado del modo indica qué comando
 * está activo; en update() un switch aplica la CarAction correspondiente.
 */
enum class RcModeState
{
  STOPPED,
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT
};

class RcMode : public Mode
{
public:
  RcMode();
  void startMode() override;
  void stopMode(OutputData& outputData) override;
  bool update(const InputData& inputData, OutputData& outputData) override;
  /** Actualiza el estado del modo con el comando recibido desde la web (callback POST /command). */
  void onWebCommandReceived(const char* action, int speed, unsigned long timestamp);

private:
  RcModeState currentState                      = RcModeState::STOPPED;
  RcModeState previousState                      = RcModeState::STOPPED;
  uint8_t lastSpeed                              = 0;
  unsigned long lastWebCommandTime               = 0;
  static constexpr unsigned long kCommandTimeoutMs = 400;
  bool webCommandActive;
  bool stopFromWeb = false; // true = comando "stop" desde web (forceStop), false = timeout (freeStop)
};
