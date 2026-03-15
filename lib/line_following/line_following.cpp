#include "line_following.h"

#include <Arduino.h>

LineFollowingMode::LineFollowingMode()
  : lastLineState(LineState::LOST)
  , isPulseActive(false)
  , pulseStartTime(0)
  , correctionPulseDone(false)
{
}

void LineFollowingMode::startMode()
{
  lastLineState       = LineState::LOST;
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

// Solo estados donde aplicamos giro en pulso (LOST_LEFT/LOST_RIGHT se tratan aparte, giro entero)
static bool isPulseCorrectionState(LineState s)
{
  return s == LineState::CENTER_LEFT || s == LineState::LEFT || s == LineState::CENTER_RIGHT || s == LineState::RIGHT;
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

  unsigned long now = millis();

  // 3. CENTER o LOST: sin pulso, acción directa
  if (currentLineState == LineState::CENTER)
  {
    isPulseActive       = false;
    correctionPulseDone = false;
    CarActions::forward(outputData, SPEED_STRAIGHT);
    return;
  }
  if (currentLineState == LineState::LOST)
  {
    isPulseActive       = false;
    correctionPulseDone = false;
    CarActions::forceStop(outputData);
    return;
  }

  // 3b. LOST_LEFT / LOST_RIGHT: giro entero (sin pulsos) hasta recuperar la línea
  if (currentLineState == LineState::LOST_LEFT)
  {
    isPulseActive       = false;
    correctionPulseDone = false;
    CarActions::turnLeft(outputData, SPEED);
    return;
  }
  if (currentLineState == LineState::LOST_RIGHT)
  {
    isPulseActive       = false;
    correctionPulseDone = false;
    CarActions::turnRight(outputData, SPEED);
    return;
  }

  // 4. Estado de corrección (CENTER_LEFT, LEFT, CENTER_RIGHT, RIGHT): giro en pulso
  if (isPulseCorrectionState(currentLineState))
  {
    if (!isPulseActive)
    {
      isPulseActive  = true;
      pulseStartTime = now;
    }

    if (now - pulseStartTime < PULSE_MS)
    {
      switch (currentLineState)
      {
        case LineState::CENTER_LEFT:
        case LineState::LEFT:
          CarActions::turnLeft(outputData, SPEED);
          break;
        case LineState::CENTER_RIGHT:
        case LineState::RIGHT:
          CarActions::turnRight(outputData, SPEED);
          break;
        default:
          break;
      }
      return;
    }
    isPulseActive       = false;
    correctionPulseDone = true;
    CarActions::forward(outputData, SPEED_STRAIGHT);
    return;
  }
  // Si salimos de estado de corrección, resetear para el próximo ciclo
  isPulseActive       = false;
  correctionPulseDone = false;
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