#ifndef wifi_ap_h
#define wifi_ap_h

#include "elegoo_smartcar_lib.h"
#include "esp_camera.h"

#include <WebServer.h>
#include <WiFi.h>
#include <functional>

class WiFiAP
{
public:
  void init(void);
  void loop(void);
  void setCommandCallback(std::function<void(const char*, int)> cb);

  String wifi_name;
  String wifi_ip;
  WebServer server; // Público para que Streaming pueda acceder

private:
  const char* ssid     = "ESP32-CAM";
  const char* password = "12345678";

  // void setup_camera(void);
  void setup_wifi(void);
  void setup_server(void);
  void handle_root(void);
  void handle_ping(void);
  void handle_command(void);

  std::function<void(const char*, int)> commandCallback;
};

#endif
