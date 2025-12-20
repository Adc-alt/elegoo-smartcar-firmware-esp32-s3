#ifndef wifi_ap_h
#define wifi_ap_h

#include "elegoo_smartcar_lib.h"
#include "esp_camera.h"

#include <WebServer.h>
#include <WiFi.h>


class WiFiAP
{
public:
  void init(void);
  void loop(void);

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
};

#endif
