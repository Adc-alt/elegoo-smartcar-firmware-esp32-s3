#pragma once

#include "../car_actions/car_actions.h"
#include "../inputs/inputs.h"
#include "../mode_manager/mode_manager.h"
#include "../outputs/outputs.h"

enum IR_STATUS
{
  IR_IDLE,
  IR_FORWARD,
  IR_BACKWARD,
  IR_LEFT,
  IR_RIGHT,
  IR_STOP,
  IR_SERVO_LEFT,
  IR_SERVO_RIGHT,
  IR_SERVO_CENTER,
};

class IrMode : public Mode
{
public:
  IrMode();
  void startMode() override;
  void stopMode(OutputData& outputData) override;
  bool update(const InputData& inputData, OutputData& outputData) override;
  // const char* getName() const override

  // {
  //   return "IR_MODE";
  // }

private:
  void processIrCommand(unsigned long irRaw, OutputData& outputData);

  // Temporizador para comando IR
  unsigned long lastCommandTime                 = 0; // Timestamp
  static const unsigned long COMMAND_TIMEOUT_MS = 400;
  bool commandActive;
};

String statusToString(IR_STATUS status);