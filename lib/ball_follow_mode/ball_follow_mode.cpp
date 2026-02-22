// lib/ball_follow_mode/ball_follow_mode.cpp
#include "ball_follow_mode.h"

#include <Arduino.h>

BallFollowMode::BallFollowMode()
  : ballDetected(false)
  , ballCenterX(0)
  , ballCenterY(0)
  , lastDifferentialTime(0)
  , differentialActive(false)
{
}

void BallFollowMode::startMode()
{
  ballDetected         = false;
  ballCenterX          = 0;
  ballCenterY          = 0;
  lastDifferentialTime = 0;
  differentialActive  = false;
}

void BallFollowMode::stopMode(OutputData& outputData)
{
  ballDetected        = false;
  differentialActive = false;
  outputData.useDifferentialMotors = false;
  CarActions::forceStop(outputData);
  CarActions::setServoAngle(outputData, 90);
}

void BallFollowMode::onDifferentialReceived(unsigned long timestamp)
{
  lastDifferentialTime = timestamp;
  differentialActive  = true;
}

bool BallFollowMode::update(const InputData& inputData, OutputData& outputData)
{
  if (differentialActive && (millis() - lastDifferentialTime >= DIFFERENTIAL_TIMEOUT_MS))
  {
    outputData.useDifferentialMotors = false;
    CarActions::freeStop(outputData);
    differentialActive = false;
  }

  // TODO: Implementar análisis de imagen para detectar bola verde
  ballDetected = false;
  return differentialActive;
}
