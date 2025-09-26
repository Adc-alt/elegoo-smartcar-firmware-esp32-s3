#ifndef _CameraStreaming_AP_H
#define _CameraStreaming_AP_H

#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

class CameraStreaming_AP
{
public:
  void init(void);
  void loop(void);
  
  String wifi_name;
  String wifi_ip;

private:
  const char* ssid = "ESP32-CAM";
  const char* password = "12345678";
  
  WebServer server;
  
  void setup_camera(void);
  void setup_wifi(void);
  void setup_server(void);
  void handle_root(void);
  void handle_stream(void);
  void handle_capture(void);
};

#endif
