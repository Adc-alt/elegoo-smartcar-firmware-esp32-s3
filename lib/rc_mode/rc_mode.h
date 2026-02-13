#pragma once

#include "../car_actions/car_actions.h"
#include "../inputs/inputs.h"
#include "../mode_manager/mode_manager.h"
#include "../outputs/outputs.h"

/**
 * Modo de control remoto por web/WiFi (equivalente a IR_MODE pero desde móvil/PC).
 * Recibe comandos a través de POST /command y los ejecuta con timeout de 400ms.
 */
class RcMode : public Mode
{
public:
  RcMode();
  void startMode() override;
  void stopMode(OutputData& outputData) override;
  bool update(const InputData& inputData, OutputData& outputData) override;
  void onWebCommandReceived(const char* action, unsigned long timestamp);

private:
  // Timeout para comandos web (igual que IR: tras 400ms sin nuevo comando se hace freeStop)
  unsigned long lastWebCommandTime              = 0;
  static const unsigned long COMMAND_TIMEOUT_MS = 400;
  bool webCommandActive;
};
