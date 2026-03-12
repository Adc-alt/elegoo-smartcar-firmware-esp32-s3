#include "line_following.h"

#include <Arduino.h>

LineFollowingMode::LineFollowingMode() : lastLineState(LineState::LOST) {}

void LineFollowingMode::startMode()
{
  lastLineState = LineState::LOST;
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

void LineFollowingMode::updateLogic(const InputData& inputData, OutputData& outputData)
{
  // 1. Determinar el estado actual de la línea (LineState)
  LineState currentLineState = determineLineState(inputData);

  // 2. Actualizar lastLineState si se detecta línea (para estados LOST_*)
  if (currentLineState != LineState::LOST_LEFT && currentLineState != LineState::LOST_RIGHT &&
      currentLineState != LineState::LOST)
  {
    lastLineState = currentLineState;
  }

  // 3. Aplicar acción al coche según LineState (un solo switch)
  switch (currentLineState)
  {
    case LineState::CENTER:
      CarActions::forward(outputData, SPEED_STRAIGHT);
      break;

    case LineState::CENTER_LEFT:
    case LineState::LEFT:
      CarActions::turnLeft(outputData, SPEED);
      break;

    case LineState::CENTER_RIGHT:
    case LineState::RIGHT:
      CarActions::turnRight(outputData, SPEED);
      break;

    case LineState::LOST_LEFT:
      // Línea perdida, último visto a la izquierda → corregir hacia la derecha
      CarActions::turnLeft(outputData, SPEED);
      break;

    case LineState::LOST_RIGHT:
      // Línea perdida, último visto a la derecha → corregir hacia la izquierda
      CarActions::turnRight(outputData, SPEED);
      break;

    case LineState::LOST:
      CarActions::forceStop(outputData);
      break;
  }
}

LineState LineFollowingMode::determineLineState(const InputData& inputData)
{
  // Convertir valores analógicos a digitales
  // Sensor medio: valores altos (> LINE_THRESHOLD) = línea negra detectada
  // Sensores laterales: valores bajos (< LINE_THRESHOLD) = línea negra detectada
  bool leftDetected   = inputData.lineSensorLeft > LINE_THRESHOLD;
  bool middleDetected = inputData.lineSensorMiddle > LINE_THRESHOLD;
  bool rightDetected  = inputData.lineSensorRight > LINE_THRESHOLD;

  // Determinar estado según combinación de sensores
  if (leftDetected && !middleDetected && !rightDetected)
  {
    Serial.println("LineState: LEFT");
    return LineState::LEFT; // 100
  }
  if (!leftDetected && middleDetected && !rightDetected)
  {
    Serial.println("LineState: CENTER");
    return LineState::CENTER; // 010
  }
  if (!leftDetected && !middleDetected && rightDetected)
  {
    Serial.println("LineState: RIGHT");
    return LineState::RIGHT; // 001
  }
  if (leftDetected && middleDetected && !rightDetected)
  {
    Serial.println("LineState: CENTER_LEFT");
    return LineState::CENTER_LEFT; // 110
  }
  if (!leftDetected && middleDetected && rightDetected)
  {
    Serial.println("LineState: CENTER_RIGHT");
    return LineState::CENTER_RIGHT; // 011
  }
  if (leftDetected && middleDetected && rightDetected)
  {
    // Todos detectan (111) - tratar como centro
    Serial.println("LineState: NO DETECTED");
    return LineState::LOST;
  }

  // No se detecta línea (000):
  // - si veníamos de izquierda, devolver LOST_LEFT (memoria)
  // - si veníamos de derecha, devolver LOST_RIGHT (memoria)
  // - si no hay memoria clara, LOST
  if (lastLineState == LineState::LEFT || lastLineState == LineState::CENTER_LEFT)
  {
    Serial.println("LineState: LOST_LEFT");
    return LineState::LOST_LEFT;
  }
  if (lastLineState == LineState::RIGHT || lastLineState == LineState::CENTER_RIGHT)
  {
    Serial.println("LineState: LOST_RIGHT");
    return LineState::LOST_RIGHT;
  }
  return LineState::LOST;
}