#include "line_following.h"

#include <Arduino.h>

LineFollowingMode::LineFollowingMode()
  : currentState(LineFollowingModeState::IDLE)
  , lastLineState(LineState::LOST)
{
}

void LineFollowingMode::startMode()
{
  // Resetear estado al iniciar el modo
  currentState  = LineFollowingModeState::IDLE;
  lastLineState = LineState::LOST;
  Serial.println("LineFollowingMode: Modo iniciado");
}

void LineFollowingMode::stopMode(OutputData& outputData)
{
  // Parar el coche al salir del modo
  CarActions::freeStop(outputData);
  CarActions::setServoAngle(outputData, 90);
  Serial.println("LineFollowingMode: Modo detenido");
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

  // 3. Switch FSM: cambiar estado del modo según el estado de la línea detectado
  switch (currentLineState)
  {
    case LineState::CENTER:
      currentState = LineFollowingModeState::MOVING_FORWARD;
      break;

    case LineState::CENTER_LEFT:
      currentState = LineFollowingModeState::TURNING_LEFT;
      break;

    case LineState::CENTER_RIGHT:
      currentState = LineFollowingModeState::TURNING_RIGHT;
      break;

    case LineState::LEFT:
      currentState = LineFollowingModeState::SMOOTH_TURNING_LEFT;
      break;

    case LineState::RIGHT:
      currentState = LineFollowingModeState::SMOOTH_TURNING_RIGHT;
      break;

    case LineState::LOST_LEFT:
      currentState = LineFollowingModeState::TURNING_LEFT;
      break;

    case LineState::LOST_RIGHT:
      currentState = LineFollowingModeState::TURNING_RIGHT;
      break;

    case LineState::LOST:
      currentState = LineFollowingModeState::IDLE;
      break;
  }

  // 4. Switch de ejecución: ejecutar acciones según el estado del modo
  switch (currentState)
  {
    case LineFollowingModeState::IDLE:
      CarActions::freeStop(outputData);
      break;

    case LineFollowingModeState::MOVING_FORWARD:
      CarActions::forward(outputData, SPEED);
      break;

    case LineFollowingModeState::SMOOTH_TURNING_LEFT:
      CarActions::turnLeft(outputData, SPEED - 10);
      break;

    case LineFollowingModeState::SMOOTH_TURNING_RIGHT:
      CarActions::turnRight(outputData, SPEED - 10);
      break;

    case LineFollowingModeState::TURNING_LEFT:
      CarActions::turnLeft(outputData, SPEED);
      break;

    case LineFollowingModeState::TURNING_RIGHT:
      CarActions::turnRight(outputData, SPEED);
      break;
  }
}

LineState LineFollowingMode::determineLineState(const InputData& inputData)
{
  // Convertir valores analógicos a digitales (0 = línea detectada, 1 = no detectada)
  // Valores bajos (< LINE_THRESHOLD) = línea negra detectada
  bool leftDetected   = inputData.lineSensorLeft < LINE_THRESHOLD;
  bool middleDetected = inputData.lineSensorMiddle < LINE_THRESHOLD;
  bool rightDetected  = inputData.lineSensorRight < LINE_THRESHOLD;

  // Determinar estado según combinación de sensores
  if (leftDetected && !middleDetected && !rightDetected)
  {
    return LineState::CENTER_LEFT; // 100
  }
  if (!leftDetected && middleDetected && !rightDetected)
  {
    return LineState::CENTER; // 010
  }
  if (!leftDetected && !middleDetected && rightDetected)
  {
    return LineState::CENTER_RIGHT; // 001
  }
  if (leftDetected && middleDetected && !rightDetected)
  {
    return LineState::LEFT; // 110
  }
  if (!leftDetected && middleDetected && rightDetected)
  {
    return LineState::RIGHT; // 011
  }
  if (leftDetected && middleDetected && rightDetected)
  {
    return LineState::CENTER; // 111 -> todos detectados, tratar como centro
  }

  // No se detecta línea (000) - usar ternario para determinar dirección perdida
  return (lastLineState == LineState::LEFT || lastLineState == LineState::CENTER_LEFT)
           ? LineState::LOST_LEFT
           : ((lastLineState == LineState::RIGHT || lastLineState == LineState::CENTER_RIGHT) ? LineState::LOST_RIGHT
                                                                                              : LineState::LOST);
}
