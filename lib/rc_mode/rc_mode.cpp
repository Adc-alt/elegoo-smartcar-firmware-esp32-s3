// lib/rc_mode/rc_mode.cpp
#include "rc_mode.h"

#include <Arduino.h>

RcMode::RcMode()
  : webCommandActive(false)
  , lastWebCommandTime(0)
{
}

void RcMode::startMode()
{
  webCommandActive   = false;
  lastWebCommandTime = 0;
}

void RcMode::stopMode(OutputData& outputData)
{
  webCommandActive   = false;
  lastWebCommandTime = 0;
  CarActions::forceStop(outputData);
  CarActions::setServoAngle(outputData, 90);
}

bool RcMode::update(const InputData& inputData, OutputData& outputData)
{
  // Los comandos web se procesan en el callback de webHost.setCommandCallback()
  // Este modo solo gestiona el timeout: si no hay comando activo reciente, hacer freeStop
  unsigned long currentTime = millis();

  if (webCommandActive)
  {
    unsigned long elapsedTime = currentTime - lastWebCommandTime;

    if (elapsedTime >= COMMAND_TIMEOUT_MS)
    {
      CarActions::freeStop(outputData);
      webCommandActive = false;
    }
  }

  return webCommandActive;
}

void RcMode::onWebCommandReceived(const char* action, unsigned long timestamp)
{
  lastWebCommandTime = timestamp;
  webCommandActive   = (action != nullptr && strcmp(action, "stop") != 0);
}