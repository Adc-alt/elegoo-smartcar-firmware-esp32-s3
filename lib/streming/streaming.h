#ifndef STREAMING_H
#define STREAMING_H

#include "esp_camera.h"
#include <WebServer.h>

class Streaming {
public:
  void init(WebServer* server);
  void loop();
  
  void handle_stream();
  void handle_capture();
  
private:
  WebServer* webServer;
};

#endif
