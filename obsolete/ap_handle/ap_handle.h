#ifndef CAMERA_STREAMING_AP_H
#define CAMERA_STREAMING_AP_H

#include "esp_camera.h"

#include <WebServer.h>
#include <WiFi.h>

class CameraStreaming_AP
{
public:
  void init(void);
  void loop(void);

  String wifiName;
  String wifiIp;

private:
  static constexpr const char* kApSsid     = "ESP32-CAM";
  static constexpr const char* kApPassword = "12345678";

  WebServer server;

  void setupCamera(void);
  void setupWifi(void);
  void setupServer(void);
  void handleRoot(void);
  void handleStream(void);
  void handleCapture(void);
};

#endif
