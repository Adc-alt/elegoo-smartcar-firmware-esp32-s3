#include "line_following.h"

#include <Arduino.h>

LineFollowingMode::LineFollowingMode()
  : currentState(LineFollowingModeState::IDLE)
  , previousState(LineFollowingModeState::IDLE)
  , lastLineState(LineState::LOST)
  , turnPulseStartTime(0)
  , isTurnPulseActive(false)
  , isLostRecoveryPulse(false)
  , lostRecoveryConsumed(false)
  , isCenterLeftTwoPhase(false)
  , isCenterRightTwoPhase(false)
// , forwardStabilizationStartTime(0)
// , isForwardStabilizationActive(false)
{
}

void LineFollowingMode::startMode()
{
  // Resetear estado al iniciar el modo
  currentState          = LineFollowingModeState::IDLE;
  previousState         = LineFollowingModeState::IDLE;
  lastLineState         = LineState::LOST;
  turnPulseStartTime    = 0;
  isTurnPulseActive     = false;
  isLostRecoveryPulse   = false;
  lostRecoveryConsumed  = false;
  isCenterLeftTwoPhase  = false;
  isCenterRightTwoPhase = false;
  // forwardStabilizationStartTime = 0;
  // isForwardStabilizationActive  = false;
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
  else
  {
    // Si seguimos en 0 0 0 (cualquier LOST_*), no resetear lastLineState aquí.
  }

  // Si volvemos a detectar línea (no LOST_*), permitir otra recuperación futura
  if (currentLineState != LineState::LOST_LEFT && currentLineState != LineState::LOST_RIGHT &&
      currentLineState != LineState::LOST)
  {
    lostRecoveryConsumed = false;
  }

  unsigned long currentTime = millis();

  // 3. Estado siempre según la lectura actual: me da igual si hay pulso activo
  // 100/110 → izquierda, 001/011 → derecha, 010/111 → forward, 000 → parar o recuperación
  switch (currentLineState)
  {
    case LineState::CENTER:
      currentState          = LineFollowingModeState::MOVING_FORWARD;
      isTurnPulseActive     = false;
      isCenterLeftTwoPhase  = false;
      isCenterRightTwoPhase = false;
      break;

    case LineState::CENTER_LEFT:
    case LineState::LEFT:
      currentState = (currentLineState == LineState::LEFT) ? LineFollowingModeState::TURNING_LEFT
                                                           : LineFollowingModeState::SMOOTH_TURNING_LEFT;
      // isCenterLeftTwoPhase  = (currentLineState == LineState::CENTER_LEFT); // [two-phase]
      isCenterLeftTwoPhase = false;
      if (previousState != currentState || !isTurnPulseActive)
      {
        turnPulseStartTime = currentTime;
        if (!isTurnPulseActive && currentState == LineFollowingModeState::SMOOTH_TURNING_LEFT)
          CarActions::turnRight(outputData, SPEED); // Nuevo pulso tras reset
        if (!isTurnPulseActive && currentState == LineFollowingModeState::TURNING_LEFT)
          CarActions::turnLeft(outputData, SPEED); // Nuevo pulso giro fuerte izquierda (100)
      }
      isTurnPulseActive     = true;
      isLostRecoveryPulse   = false;
      isCenterRightTwoPhase = false;
      break;

    case LineState::CENTER_RIGHT:
    case LineState::RIGHT:
      currentState = (currentLineState == LineState::RIGHT) ? LineFollowingModeState::TURNING_RIGHT
                                                            : LineFollowingModeState::SMOOTH_TURNING_RIGHT;
      // isCenterRightTwoPhase = (currentLineState == LineState::CENTER_RIGHT); // [two-phase]
      isCenterRightTwoPhase = false;
      if (previousState != currentState || !isTurnPulseActive)
      {
        turnPulseStartTime = currentTime;
        if (!isTurnPulseActive && currentState == LineFollowingModeState::SMOOTH_TURNING_RIGHT)
          CarActions::turnLeft(outputData, SPEED); // Nuevo pulso tras reset
        if (!isTurnPulseActive && currentState == LineFollowingModeState::TURNING_RIGHT)
          CarActions::turnRight(outputData, SPEED); // Nuevo pulso giro fuerte derecha (001)
      }
      isTurnPulseActive    = true;
      isLostRecoveryPulse  = false;
      isCenterLeftTwoPhase = false;
      break;

    case LineState::LOST_LEFT:
      // Estábamos en 1 0 0 (o 1 1 0), pasamos a 0 0 0 → perdidos por la derecha → pulso fuerte a la izquierda
      if (!lostRecoveryConsumed)
      {
        currentState         = LineFollowingModeState::TURNING_RIGHT;
        turnPulseStartTime   = currentTime;
        isTurnPulseActive    = true;
        isLostRecoveryPulse  = true;
        lostRecoveryConsumed = true;
      }
      else
      {
        currentState = LineFollowingModeState::IDLE;
      }
      break;

    case LineState::LOST_RIGHT:
      // Estábamos en 0 0 1 (o 0 1 1), pasamos a 0 0 0 → perdidos por la izquierda → pulso fuerte a la derecha
      if (!lostRecoveryConsumed)
      {
        currentState         = LineFollowingModeState::TURNING_LEFT;
        turnPulseStartTime   = currentTime;
        isTurnPulseActive    = true;
        isLostRecoveryPulse  = true;
        lostRecoveryConsumed = true;
      }
      else
      {
        currentState = LineFollowingModeState::IDLE;
      }
      break;

    case LineState::LOST:
      currentState        = LineFollowingModeState::IDLE;
      isTurnPulseActive   = false;
      isLostRecoveryPulse = false;
      break;
  }

  // Solo para recuperación LOST: cuando el pulso de recuperación ha terminado, parar
  if (isTurnPulseActive && isLostRecoveryPulse && (currentTime - turnPulseStartTime >= LOST_RECOVERY_PULSE_MS))
  {
    isTurnPulseActive   = false;
    isLostRecoveryPulse = false;
    currentState        = LineFollowingModeState::IDLE;
  }

  // 4. Solo al cambiar de estado: imprimir y enviar UN solo comando al ATmega (mínimo de pulsos para pruebas)
  if (currentState != previousState)
  {
    switch (currentState)
    { // NO ME CAMBIES ESTO CHATGPT
      case LineFollowingModeState::IDLE:
        Serial.println("LineFollowingMode: IDLE");
        CarActions::forceStop(outputData);
        break;

      case LineFollowingModeState::MOVING_FORWARD:
        Serial.println("LineFollowingMode: MOVING_FORWARD");
        CarActions::forward(outputData, SPEED);
        break;

      case LineFollowingModeState::SMOOTH_TURNING_LEFT:
        Serial.println("LineFollowingMode: SMOOTH_TURNING_LEFT");
        // [two-phase] if (isCenterLeftTwoPhase) CarActions::forward(outputData, SPEED); else
        CarActions::turnLeft(outputData, SPEED);
        break;

      case LineFollowingModeState::SMOOTH_TURNING_RIGHT:
        Serial.println("LineFollowingMode: SMOOTH_TURNING_RIGHT");
        // [two-phase] if (isCenterRightTwoPhase) CarActions::forward(outputData, SPEED); else
        CarActions::turnRight(outputData, SPEED);
        break;

      case LineFollowingModeState::TURNING_LEFT:
        Serial.println("LineFollowingMode: TURNING_LEFT");
        CarActions::turnLeft(outputData, SPEED);
        break;

      case LineFollowingModeState::TURNING_RIGHT:
        Serial.println("LineFollowingMode: TURNING_RIGHT");
        CarActions::turnRight(outputData, SPEED);
        break;
    }
    previousState = currentState;
  }
  else
  {
    // Sin cambio de estado: en 0 1 0 seguir enviando forward
    if (currentState == LineFollowingModeState::MOVING_FORWARD)
      CarActions::forward(outputData, SPEED);
    else if ((currentState == LineFollowingModeState::TURNING_LEFT ||
              currentState == LineFollowingModeState::TURNING_RIGHT) &&
             (currentTime - turnPulseStartTime < (isLostRecoveryPulse ? LOST_RECOVERY_PULSE_MS : STRONG_TURN_PULSE_MS)))
    {
      if (isLostRecoveryPulse)
      {
        // Recuperación LOST: enviar pulsos de giro
        if (currentState == LineFollowingModeState::TURNING_LEFT)
          CarActions::turnLeft(outputData, SPEED);
        else
          CarActions::turnRight(outputData, SPEED);
      }
      // Giros fuertes normales (100 / 001): durante el pulso no reenviar (ya se envió al iniciar pulso)
    }
    else if ((currentState == LineFollowingModeState::TURNING_LEFT ||
              currentState == LineFollowingModeState::TURNING_RIGHT) &&
             !isLostRecoveryPulse && (currentTime - turnPulseStartTime >= STRONG_TURN_PULSE_MS))
    {
      // Pulso fuerte terminado (100/001): resetear para permitir nuevo pulso si seguimos en turn left/right
      isTurnPulseActive = false;
      CarActions::freeStop(outputData);
    }
    // [two-phase] ir adelante + girar para CENTER_LEFT/RIGHT
    // else if (currentState == LineFollowingModeState::SMOOTH_TURNING_LEFT && isCenterLeftTwoPhase)
    // {
    //   unsigned long elapsed = currentTime - turnPulseStartTime;
    //   if (elapsed < CENTER_FORWARD_PULSE_MS)
    //     CarActions::forward(outputData, SPEED);
    //   else if (elapsed < CENTER_FORWARD_PULSE_MS + SMOOTH_TURN_PULSE_MS)
    //     CarActions::turnRight(outputData, SPEED);
    //   else
    //   {
    //     isTurnPulseActive = false;
    //     CarActions::freeStop(outputData);
    //   }
    // }
    // else if (currentState == LineFollowingModeState::SMOOTH_TURNING_RIGHT && isCenterRightTwoPhase)
    // {
    //   unsigned long elapsed = currentTime - turnPulseStartTime;
    //   if (elapsed < CENTER_FORWARD_PULSE_MS)
    //     CarActions::forward(outputData, SPEED);
    //   else if (elapsed < CENTER_FORWARD_PULSE_MS + SMOOTH_TURN_PULSE_MS)
    //     CarActions::turnLeft(outputData, SPEED);
    //   else
    //   {
    //     isTurnPulseActive = false;
    //     CarActions::freeStop(outputData);
    //   }
    // }
    else if ((currentState == LineFollowingModeState::SMOOTH_TURNING_LEFT ||
              currentState == LineFollowingModeState::SMOOTH_TURNING_RIGHT) &&
             (currentTime - turnPulseStartTime < SMOOTH_TURN_PULSE_MS))
    {
      // Dentro del pulso suave (110 / 011): no enviar nada
    }
    else if ((currentState == LineFollowingModeState::SMOOTH_TURNING_LEFT ||
              currentState == LineFollowingModeState::SMOOTH_TURNING_RIGHT) &&
             (currentTime - turnPulseStartTime >= SMOOTH_TURN_PULSE_MS))
    {
      // Pulso suave terminado: resetear para permitir nuevo pulso si seguimos en 110/011
      isTurnPulseActive = false;
      CarActions::freeStop(outputData);
    }
    else
      CarActions::freeStop(outputData);
  }

  // Envío continuo cada frame (comentado): solo queremos 1 pulso por cambio de estado para ver respuesta real
  // switch (currentState)
  // {
  //   case LineFollowingModeState::IDLE:
  //     CarActions::forceStop(outputData);
  //     break;
  //   case LineFollowingModeState::MOVING_FORWARD:
  //     CarActions::forward(outputData, SPEED);
  //     break;
  //   case LineFollowingModeState::SMOOTH_TURNING_LEFT:
  //     if (isTurnPulseActive) CarActions::turnLeft(outputData, SPEED - 17);
  //     else CarActions::forward(outputData, SPEED);
  //     break;
  //   case LineFollowingModeState::SMOOTH_TURNING_RIGHT:
  //     if (isTurnPulseActive) CarActions::turnRight(outputData, SPEED - 17);
  //     else CarActions::forward(outputData, SPEED);
  //     break;
  //   case LineFollowingModeState::TURNING_LEFT:
  //     if (isTurnPulseActive) CarActions::turnLeft(outputData, SPEED - 10);
  //     else CarActions::forward(outputData, SPEED);
  //     break;
  //   case LineFollowingModeState::TURNING_RIGHT:
  //     if (isTurnPulseActive) CarActions::turnRight(outputData, SPEED - 10);
  //     else CarActions::forward(outputData, SPEED);
  //     break;
  // }
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
    return LineState::LEFT; // 100
  }
  if (!leftDetected && middleDetected && !rightDetected)
  {
    return LineState::CENTER; // 010
  }
  if (!leftDetected && !middleDetected && rightDetected)
  {
    return LineState::RIGHT; // 001
  }
  if (leftDetected && middleDetected && !rightDetected)
  {
    return LineState::CENTER_LEFT; // 110
  }
  if (!leftDetected && middleDetected && rightDetected)
  {
    return LineState::CENTER_RIGHT; // 011
  }
  if (leftDetected && middleDetected && rightDetected)
  {
    // Todos detectan (111) - tratar como centro
    return LineState::CENTER;
  }

  // No se detecta línea (000):
  // - si veníamos de izquierda, devolver LOST_LEFT (memoria)
  // - si veníamos de derecha, devolver LOST_RIGHT (memoria)
  // - si no hay memoria clara, LOST
  if (lastLineState == LineState::LEFT || lastLineState == LineState::CENTER_LEFT)
  {
    return LineState::LOST_LEFT;
  }
  if (lastLineState == LineState::RIGHT || lastLineState == LineState::CENTER_RIGHT)
  {
    return LineState::LOST_RIGHT;
  }
  return LineState::LOST;
}
