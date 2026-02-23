#include "car_actions.h"

#include <Arduino.h>

// Inicializar estado global
CarStatus CarActions::currentStatus = {"", 0, 90, ""};

void CarActions::forward(OutputData& outputData, uint8_t speed)
{
  // Solo imprimir si cambia la acción
  // if (currentStatus.currentAction != "forward")
  // {
  //   Serial.println((String) "CarActions: FORWARD - speed: " + speed);
  // }

  outputData.leftAction  = "forward";
  outputData.leftSpeed   = speed;
  outputData.rightAction = "forward";
  outputData.rightSpeed  = speed;

  // Actualizar estado global
  currentStatus.currentAction = "forward";
  currentStatus.currentSpeed  = speed;
}

void CarActions::backward(OutputData& outputData, uint8_t speed)
{
  // Solo imprimir si cambia la acción
  // if (currentStatus.currentAction != "backward")
  // {
  //   Serial.println((String) "CarActions: BACKWARD - speed: " + speed);
  // }
  outputData.leftAction  = "backward";
  outputData.leftSpeed   = speed;
  outputData.rightAction = "backward";
  outputData.rightSpeed  = speed;

  // Actualizar estado global
  currentStatus.currentAction = "backward";
  currentStatus.currentSpeed  = speed;
}

void CarActions::turnLeft(OutputData& outputData, uint8_t speed)
{
  // // Solo imprimir si cambia la acción
  // if (currentStatus.currentAction != "turnLeft")
  // {
  //   Serial.println((String) "CarActions: TURN LEFT - speed: " + speed);
  // }

  outputData.leftAction  = "turnLeft";
  outputData.leftSpeed   = speed;
  outputData.rightAction = "turnLeft";
  outputData.rightSpeed  = speed;

  // Actualizar estado global
  currentStatus.currentAction = "turnLeft";
  currentStatus.currentSpeed  = speed;
}

void CarActions::turnRight(OutputData& outputData, uint8_t speed)
{
  // Solo imprimir si cambia la acción
  //   if (currentStatus.currentAction != "turnRight")
  // {
  //   Serial.println((String) "CarActions: TURN RIGHT - speed: " + speed);
  // }

  outputData.leftAction  = "turnRight";
  outputData.leftSpeed   = speed;
  outputData.rightAction = "turnRight";
  outputData.rightSpeed  = speed;

  // Actualizar estado global
  currentStatus.currentAction = "turnRight";
  currentStatus.currentSpeed  = speed;
}

void CarActions::setDifferential(OutputData& outputData, uint8_t leftSpeed, uint8_t rightSpeed)
{
  outputData.leftAction  = "forward";
  outputData.leftSpeed   = leftSpeed;
  outputData.rightAction = "forward";
  outputData.rightSpeed  = rightSpeed;
  currentStatus.currentAction = "forward";
  currentStatus.currentSpeed  = (leftSpeed + rightSpeed) / 2; // representativo
}

void CarActions::freeStop(OutputData& outputData)
{
  // Solo imprimir si cambia la acción
  // if (currentStatus.currentAction != "freeStop")
  // {
  //   Serial.println("CarActions: FREE STOP");
  // }

  outputData.leftAction  = "freeStop";
  outputData.leftSpeed   = 0;
  outputData.rightAction = "freeStop";
  outputData.rightSpeed  = 0;

  // Actualizar estado global
  currentStatus.currentAction = "freeStop";
  currentStatus.currentSpeed  = 0;
}

void CarActions::forceStop(OutputData& outputData)
{
  // Solo imprimir si cambia la acción
  //  if (currentStatus.currentAction != "forceStop")
  // {
  //   Serial.println("CarActions: FORCE STOP");
  // }

  outputData.leftAction  = "forceStop";
  outputData.leftSpeed   = 0;
  outputData.rightAction = "forceStop";
  outputData.rightSpeed  = 0;

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