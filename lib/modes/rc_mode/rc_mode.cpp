// lib/rc_mode/rc_mode.cpp
#include "rc_mode.h"

#include <Arduino.h>
#include <cstring>

RcMode::RcMode()
  : webCommandActive(false)
  , lastWebCommandTime(0)
{
}

void RcMode::startMode()
{
  currentState       = RcModeState::STOPPED;
  previousState      = RcModeState::STOPPED;
  webCommandActive   = false;
  lastWebCommandTime = 0;
  lastSpeed          = 0;
  stopFromWeb        = false;
}

void RcMode::stopMode(OutputData& outputData)
{
  currentState       = RcModeState::STOPPED;
  webCommandActive   = false;
  lastWebCommandTime = 0;
  lastSpeed          = 0;
  stopFromWeb        = false;
  CarActions::forceStop(outputData);
  CarActions::setServoAngle(outputData, 90);
}

bool RcMode::update(const InputData& inputData, OutputData& outputData)
{
  unsigned long currentTime = millis();

  if (webCommandActive && (currentTime - lastWebCommandTime >= kCommandTimeoutMs))
  {
    currentState     = RcModeState::STOPPED;
    webCommandActive = false;
    stopFromWeb      = false; // timeout → freeStop
  }

  if (currentState != previousState)
  {
    switch (currentState)
    {
      case RcModeState::FORWARD:
        Serial.println("RcModeState: FORWARD");
        break;
      case RcModeState::BACKWARD:
        Serial.println("RcModeState: BACKWARD");
        break;
      case RcModeState::LEFT:
        Serial.println("RcModeState: LEFT");
        break;
      case RcModeState::RIGHT:
        Serial.println("RcModeState: RIGHT");
        break;
      case RcModeState::STOPPED:
        Serial.println("RcModeState: STOPPED");
        break;
    }
    previousState = currentState;
  }

  switch (currentState)
  {
    case RcModeState::FORWARD:
      CarActions::forward(outputData, lastSpeed);
      break;
    case RcModeState::BACKWARD:
      CarActions::backward(outputData, lastSpeed);
      break;
    case RcModeState::LEFT:
      CarActions::turnLeft(outputData, lastSpeed);
      break;
    case RcModeState::RIGHT:
      CarActions::turnRight(outputData, lastSpeed);
      break;
    case RcModeState::STOPPED:
    default:
      if (stopFromWeb)
        CarActions::forceStop(outputData);
      else
        CarActions::freeStop(outputData);
      break;
  }

  return webCommandActive;
}

void RcMode::onWebCommandReceived(const char* action, int speed, unsigned long timestamp)
{
  lastWebCommandTime = timestamp;

  if (speed < 0)
    lastSpeed = 0;
  else if (speed > 255)
    lastSpeed = 255;
  else
    lastSpeed = static_cast<uint8_t>(speed);

  if (action == nullptr || strcmp(action, "stop") == 0)
  {
    currentState     = RcModeState::STOPPED;
    webCommandActive = false;
    stopFromWeb      = true; // comando "stop" desde web → forceStop
    return;
  }

  if (strcmp(action, "forward") == 0)
    currentState = RcModeState::FORWARD;
  else if (strcmp(action, "backward") == 0)
    currentState = RcModeState::BACKWARD;
  else if (strcmp(action, "left") == 0)
    currentState = RcModeState::LEFT;
  else if (strcmp(action, "right") == 0)
    currentState = RcModeState::RIGHT;
  else
    return; // acción desconocida, no cambiar estado

  webCommandActive = true;
}