#include "car_actions.h"

#include <Arduino.h>

// Inicializar estado global
CarStatus CarActions::currentStatus = {"", 0, 90, ""};

void CarActions::forward(OutputData& outputData, uint8_t speed)
{
  // Solo imprimir si cambia la acción
  if (currentStatus.currentAction != "forward")
  {
    Serial.println((String) "CarActions: FORWARD - speed: " + speed);
  }

  outputData.action = "forward";
  outputData.speed  = speed;

  // Actualizar estado global
  currentStatus.currentAction = "forward";
  currentStatus.currentSpeed  = speed;
}

void CarActions::backward(OutputData& outputData, uint8_t speed)
{
  // Solo imprimir si cambia la acción
  if (currentStatus.currentAction != "backward")
  {
    Serial.println((String) "CarActions: BACKWARD - speed: " + speed);
  }

  outputData.action = "backward";
  outputData.speed  = speed;

  // Actualizar estado global
  currentStatus.currentAction = "backward";
  currentStatus.currentSpeed  = speed;
}

void CarActions::turnLeft(OutputData& outputData, uint8_t speed)
{
  // Solo imprimir si cambia la acción
  if (currentStatus.currentAction != "turnLeft")
  {
    Serial.println((String) "CarActions: TURN LEFT - speed: " + speed);
  }

  outputData.action = "turnLeft";
  outputData.speed  = speed;

  // Actualizar estado global
  currentStatus.currentAction = "turnLeft";
  currentStatus.currentSpeed  = speed;
}

void CarActions::turnRight(OutputData& outputData, uint8_t speed)
{
  // Solo imprimir si cambia la acción
  if (currentStatus.currentAction != "turnRight")
  {
    Serial.println((String) "CarActions: TURN RIGHT - speed: " + speed);
  }

  outputData.action = "turnRight";
  outputData.speed  = speed;

  // Actualizar estado global
  currentStatus.currentAction = "turnRight";
  currentStatus.currentSpeed  = speed;
}

void CarActions::freeStop(OutputData& outputData)
{
  // Solo imprimir si cambia la acción
  if (currentStatus.currentAction != "freeStop")
  {
    Serial.println("CarActions: FREE STOP");
  }

  outputData.action = "freeStop";
  outputData.speed  = 0;

  // Actualizar estado global
  currentStatus.currentAction = "freeStop";
  currentStatus.currentSpeed  = 0;
}

void CarActions::forceStop(OutputData& outputData)
{
  // Solo imprimir si cambia la acción
  if (currentStatus.currentAction != "forceStop")
  {
    Serial.println("CarActions: FORCE STOP");
  }

  outputData.action = "forceStop";
  outputData.speed  = 0;

  // Actualizar estado global
  currentStatus.currentAction = "forceStop";
  currentStatus.currentSpeed  = 0;
}

void CarActions::setServoAngle(OutputData& outputData, uint8_t servoAngle)
{
  outputData.servoAngle = servoAngle;

  // Actualizar estado global
  currentStatus.currentServoAngle = servoAngle;
}

void CarActions::setLedColor(OutputData& outputData, const String& color)
{
  outputData.ledColor = color;

  // Actualizar estado global
  currentStatus.currentLedColor = color;
}

CarStatus CarActions::getStatus()
{
  return currentStatus;
}