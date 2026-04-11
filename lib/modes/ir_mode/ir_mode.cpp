#include "ir_mode.h"

#include "ir_remote_codes.h"

#include <Arduino.h>

IrMode::IrMode()
  : commandActive(false)
  , lastCommandTime(0)
{
}

void IrMode::startMode()
{
  commandActive   = false;
  lastCommandTime = 0;
}
void IrMode::stopMode(OutputData& outputData)
{
  commandActive   = false;
  lastCommandTime = 0;
  CarActions::forceStop(outputData);
  CarActions::setServoAngle(outputData, 90);
}

bool IrMode::update(const InputData& inputData, OutputData& outputData)
{
  unsigned long irRaw       = inputData.irRaw;
  unsigned long currentTime = millis();

  // Solo procesar si hay un comando IR válido (no cero)
  if (irRaw != 0)
  {
    processIrCommand(irRaw, outputData);
    lastCommandTime = currentTime;
    commandActive   = true;
    return true;
  }

  if (commandActive)
  {
    unsigned long elapsedTime = currentTime - lastCommandTime;

    if (elapsedTime >= kCommandTimeoutMs)
    {
      CarActions::freeStop(outputData);
      commandActive = false;
    }
  }

  return commandActive;
}

void IrMode::processIrCommand(unsigned long irRaw, OutputData& outputData)
{
  using namespace IrRemote;

  if (irRaw == kDriveForward)
  {
    CarActions::forward(outputData, 80);
  }
  else if (irRaw == kDriveBackward)
  {
    CarActions::backward(outputData, 80);
  }
  else if (irRaw == kDriveTurnLeft)
  {
    CarActions::turnLeft(outputData, 80);
  }
  else if (irRaw == kDriveTurnRight)
  {
    CarActions::turnRight(outputData, 80);
  }
  else if (irRaw == kDriveStop)
  {
    CarActions::forceStop(outputData);
  }
  else if (irRaw == kServoLeft)
  {
    CarActions::setServoAngle(outputData, 20);
  }
  else if (irRaw == kServoRight)
  {
    CarActions::setServoAngle(outputData, 160);
  }
  else if (irRaw == kServoCenter)
  {
    CarActions::setServoAngle(outputData, 90);
  }
  else
  {
    CarActions::freeStop(outputData);
  }
}

String statusToString(IR_STATUS status)
{
  switch (status)
  {
    case IR_IDLE:
      return "IR_IDLE";
    case IR_FORWARD:
      return "IR_FORWARD";
    case IR_BACKWARD:
      return "IR_BACKWARD";
    case IR_LEFT:
      return "IR_LEFT";
    case IR_RIGHT:
      return "IR_RIGHT";
    case IR_STOP:
      return "IR_STOP";
    case IR_SERVO_LEFT:
      return "IR_SERVO_LEFT";
    case IR_SERVO_RIGHT:
      return "IR_SERVO_RIGHT";
    case IR_SERVO_CENTER:
      return "IR_SERVO_CENTER";
    default:
      return "IR_UNKNOWN";
  }
}