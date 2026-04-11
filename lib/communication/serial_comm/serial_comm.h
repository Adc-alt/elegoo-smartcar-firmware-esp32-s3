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

  /** Intervalo entre envíos UART (ms); valor fijo de compilación. */
  static constexpr unsigned long kSendIntervalMs = 20;
  /** Timeout sin recibir JSON válido (ms). */
  static constexpr unsigned long kTimeoutIntervalMs = 2000;

  void initializeJsons();
  void sendJsonBySerial();
  bool readJsonBySerial();
  void checkTimeout();
};