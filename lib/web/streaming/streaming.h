#ifndef STREAMING_H
#define STREAMING_H

#include "elegoo_smartcar_lib.h"
#include "esp_camera.h"

#include <Arduino.h>
#include <WebServer.h>
#include <functional>

class Streaming
{
public:
  void init(WebServer* server, std::function<bool()> allowStream = nullptr);
  void loop();

  void setDifferentialCallback(std::function<void(const char*, uint8_t, const char*, uint8_t)> cb);

  void setup_camera();
  void handle_stream();
  void handle_differential_command();
  void handle_capture();

private:
  WebServer* webServer;
  std::function<bool()> allowStream;
  std::function<void(const char*, uint8_t, const char*, uint8_t)> differentialCallback;

  unsigned long lastFrameTime       = 0;
  const unsigned long frameInterval = 100; // 100ms entre frames (10 FPS)
};

#endif
