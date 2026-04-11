#include "line_following.h"

#include <Arduino.h>

LineFollowingMode::LineFollowingMode()
  : lastLineState(LineState::LOST)
  , currentModeState(LineFollowingModeState::LOST)
  , isPulseActive(false)
  , pulseStartTime(0)
  , correctionPulseDone(false)
{
}

void LineFollowingMode::startMode()
{
  lastLineState       = LineState::LOST;
  currentModeState    = LineFollowingModeState::LOST;
  isPulseActive       = false;
  correctionPulseDone = false;
}

void LineFollowingMode::stopMode(OutputData& outputData)
{
  // Parar el coche al salir del modo
  CarActions::freeStop(outputData);
  CarActions::setServoAngle(outputData, 90);
  // Serial.println("LineFollowingMode: Modo detenido");
}

bool LineFollowingMode::update(const InputData& inputData, OutputData& outputData)
{
  // Lógica del modo basada en estados
  updateLogic(inputData, outputData);

  // El modo está siempre activo una vez iniciado
  return true;
}

static LineFollowingModeState lineStateToModeState(LineState ls)
{
  switch (ls)
  {
    case LineState::CENTER:
      return LineFollowingModeState::ON_LINE;
    case LineState::LOST:
      return LineFollowingModeState::LOST;
    case LineState::LOST_LEFT:
      return LineFollowingModeState::RECOVERING_LEFT;
    case LineState::LOST_RIGHT:
      return LineFollowingModeState::RECOVERING_RIGHT;
    case LineState::LEFT:
    case LineState::CENTER_LEFT:
      return LineFollowingModeState::CORRECTING_LEFT;
    case LineState::RIGHT:
    case LineState::CENTER_RIGHT:
      return LineFollowingModeState::CORRECTING_RIGHT;
  }
  return LineFollowingModeState::LOST;
}

void LineFollowingMode::updateLogic(const InputData& inputData, OutputData& outputData)
{
  LineState currentLineState = determineLineState(inputData);

  if (currentLineState != LineState::LOST_LEFT && currentLineState != LineState::LOST_RIGHT &&
      currentLineState != LineState::LOST)
  {
    lastLineState = currentLineState;
  }

  currentModeState = lineStateToModeState(currentLineState);

  if (currentModeState != LineFollowingModeState::CORRECTING_LEFT &&
      currentModeState != LineFollowingModeState::CORRECTING_RIGHT)
  {
    isPulseActive       = false;
    correctionPulseDone = false;
  }

  unsigned long now = millis();

  switch (currentModeState)
  {
    case LineFollowingModeState::ON_LINE:
      Serial.println("LineFollowingModeState: ON_LINE");
      CarActions::forward(outputData, kSpeedStraight);
      break;
    case LineFollowingModeState::LOST:
      Serial.println("LineFollowingModeState: LOST");
      CarActions::forceStop(outputData);
      break;
    case LineFollowingModeState::RECOVERING_LEFT:
      Serial.println("LineFollowingModeState: RECOVERING_LEFT");
      CarActions::turnLeft(outputData, kSpeed);
      break;
    case LineFollowingModeState::RECOVERING_RIGHT:
      Serial.println("LineFollowingModeState: RECOVERING_RIGHT");
      CarActions::turnRight(outputData, kSpeed);
      break;
    case LineFollowingModeState::CORRECTING_LEFT:
      Serial.println("LineFollowingModeState: CORRECTING_LEFT - isPulseActive: " + String(isPulseActive));
      if (!isPulseActive)
      {
        isPulseActive  = true;
        pulseStartTime = now;
      }
      if (now - pulseStartTime < kPulseMs)
        CarActions::turnLeft(outputData, kSpeed);
      else
      {
        isPulseActive       = false;
        correctionPulseDone = true;
        CarActions::forward(outputData, kSpeedStraight);
      }
      break;
    case LineFollowingModeState::CORRECTING_RIGHT:
      Serial.println("LineFollowingModeState: CORRECTING_RIGHT - isPulseActive: " + String(isPulseActive));
      if (!isPulseActive)
      {
        isPulseActive  = true;
        pulseStartTime = now;
      }
      if (now - pulseStartTime < kPulseMs)
        CarActions::turnRight(outputData, kSpeed);
      else
      {
        isPulseActive       = false;
        correctionPulseDone = true;
        CarActions::forward(outputData, kSpeedStraight);
      }
      break;
  }
}

LineState LineFollowingMode::determineLineState(const InputData& inputData)
{
  // Convertir valores analógicos a digitales
  // Sensor medio: valores altos (> kLineThreshold) = línea negra detectada
  // Sensores laterales: valores bajos (< kLineThreshold) = línea negra detectada
  bool leftDetected   = inputData.lineSensorLeft > kLineThreshold;
  bool middleDetected = inputData.lineSensorMiddle > kLineThreshold;
  bool rightDetected  = inputData.lineSensorRight > kLineThreshold;

  // Determinar estado según combinación de sensores
  if (leftDetected && !middleDetected && !rightDetected)
  {
    // Serial.println("LineState: LEFT");
    return LineState::LEFT; // 100
  }
  if (!leftDetected && middleDetected && !rightDetected)
  {
    // Serial.println("LineState: CENTER");
    return LineState::CENTER; // 010
  }
  if (!leftDetected && !middleDetected && rightDetected)
  {
    // Serial.println("LineState: RIGHT");
    return LineState::RIGHT; // 001
  }
  if (leftDetected && middleDetected && !rightDetected)
  {
    // Serial.println("LineState: CENTER_LEFT");
    return LineState::CENTER_LEFT; // 110
  }
  if (!leftDetected && middleDetected && rightDetected)
  {
    // Serial.println("LineState: CENTER_RIGHT");
    return LineState::CENTER_RIGHT; // 011
  }
  if (leftDetected && middleDetected && rightDetected)
  {
    // Todos detectan (111) - tratar como centro
    // Serial.println("LineState: NO DETECTED");
    return LineState::LOST;
  }

  // No se detecta línea (000):
  // - si veníamos de izquierda, devolver LOST_LEFT (memoria)
  // - si veníamos de derecha, devolver LOST_RIGHT (memoria)
  // - si no hay memoria clara, LOST
  if (lastLineState == LineState::LEFT || lastLineState == LineState::CENTER_LEFT)
  {
    // Serial.println("LineState: LOST_LEFT");
    return LineState::LOST_LEFT;
  }
  if (lastLineState == LineState::RIGHT || lastLineState == LineState::CENTER_RIGHT)
  {
    // Serial.println("LineState: LOST_RIGHT");
    return LineState::LOST_RIGHT;
  }
  return LineState::LOST;
}