#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

class SerialComm
{
public:
  JsonDocument sendJson;
  JsonDocument receiveJson;

  unsigned long lastSendTime;
  unsigned long lastReceiveTime;
  bool timeoutActive;

  const unsigned long SEND_INTERVAL    = 20;   // 500ms
  const unsigned long TIMEOUT_INTERVAL = 2000; // 2 segundos

  void initializeJsons();
  void sendJsonBySerial();
  bool readJsonBySerial();
  void checkTimeout();
};