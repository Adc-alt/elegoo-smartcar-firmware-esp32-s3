// lib/ball_follow_mode/ball_follow_mode.cpp
#include "ball_follow_mode.h"

#include <Arduino.h>

BallFollowMode::BallFollowMode()
  : ballDetected(false)
  , ballCenterX(0)
  , ballCenterY(0)
{
}

void BallFollowMode::startMode()
{
  ballDetected = false;
  ballCenterX  = 0;
  ballCenterY  = 0;
}

void BallFollowMode::stopMode(OutputData& outputData)
{
  ballDetected = false;
  CarActions::forceStop(outputData);
  CarActions::setServoAngle(outputData, 90);
}

bool BallFollowMode::update(const InputData& inputData, OutputData& outputData)
{
  // TODO: Implementar análisis de imagen para detectar bola verde
  // Por ahora: stub que no hace nada
  ballDetected = false;
  return false;
}
