#ifndef STREAMING_H
#define STREAMING_H

#include "elegoo_smartcar_lib.h"
#include "esp_camera.h"

#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>
#include <functional>

class Streaming
{
public:
  void init(WebServer* server, std::function<bool()> allowStream = nullptr);
  void loop();

  void setupCamera();
  void handleStream();

private:
  void endStreamSession();

  WebServer* webServer = nullptr;
  std::function<bool()> allowStream;

  /** Copia del cliente HTTP del stream; se escribe desde loop() para no bloquear webHost.loop(). */
  WiFiClient streamClient;
  bool streamSessionActive = false;
  bool cameraReady         = false;

  unsigned long lastFrameTime = 0;
  /** ~12.5 FPS; menos carga WiFi que burst sin límite. */
  static constexpr unsigned long kFrameIntervalMs = 80;
};

#endif
