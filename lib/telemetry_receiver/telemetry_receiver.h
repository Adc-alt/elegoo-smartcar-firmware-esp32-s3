#pragma once
#include "telemetry_frame.h"

#include <Arduino.h>
#include <ArduinoJson.h>

/*
  TelemetryReceiver
  =================
  Recibe JSON del ATmega por Serial y actualiza el TelemetryFrame.

  Responsabilidades:
  - Leer JSON del Stream (Serial)
  - Parsear el JSON
  - Actualizar TelemetryFrame con los datos recibidos

  No hace:
  - No interpreta los datos
  - No toma decisiones
  - Solo recibe y actualiza el frame de telemetría
*/
class TelemetryReceiver
{
public:
  explicit TelemetryReceiver(Stream& in);

  // Intenta recibir telemetría (no bloqueante)
  // Retorna true si se recibió un frame nuevo
  bool tryReceive(TelemetryFrame& telemetryFrame);

  // Verificar timeout (limpiar buffer si hay timeout)
  void checkTimeout(unsigned long timeoutMs = 1000);

private:
  Stream& in;
  String buffer;
  bool processingMessage        = false;
  unsigned long lastMessageTime = 0;
};