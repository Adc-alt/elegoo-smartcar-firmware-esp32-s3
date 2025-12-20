#ifndef STREAMING_H
#define STREAMING_H

#include "elegoo_smartcar_lib.h"
#include "esp_camera.h"

#include <Arduino.h>
#include <WebServer.h>


class Streaming
{
public:
  void init(WebServer* server);
  void loop();

  void setup_camera();
  void handle_stream();
  void handle_capture();

private:
  WebServer* webServer;
  unsigned long lastFrameTime       = 0;
  const unsigned long frameInterval = 100; // 100ms entre frames (10 FPS)
};

#endif
