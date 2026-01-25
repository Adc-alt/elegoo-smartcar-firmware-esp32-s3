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

  const unsigned long SEND_INTERVAL    = 50;   // ms entre envíos al Atmega (menos carga → menos riesgo de bloqueo en Serial2)
  const unsigned long TIMEOUT_INTERVAL = 2000; // 2 segundos

  void initializeJsons();
  void sendJsonBySerial();
  bool readJsonBySerial(); // No bloqueante: usa buffer y Serial2.read() solo con available()
  void checkTimeout();

private:
  static const size_t RX_LINE_MAX = 512;
  String _rxLineBuffer; // Acumula bytes hasta \n para evitar readStringUntil (bloqueante)
};