#pragma once

#include "../mode_manager/mode_manager.h"

class IrMode : public Mode
{
public:
  IrMode();
  bool update(const InputData& inputData, OutputData& outputData) override;
  // const char* getName() const override

  // {
  //   return "IR_MODE";
  // }

private:
  void processIrCommand(unsigned long irRaw, OutputData& outputData);

  // Temporizador para comando IR
  unsigned long lastCommandTime                 = 0; // Timestamp
  bool commandActive                            = false;
  static const unsigned long COMMAND_TIMEOUT_MS = 400;
};