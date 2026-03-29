#ifndef WIFI_AP_MANAGER_H
#define WIFI_AP_MANAGER_H

#include "elegoo_smartcar_lib.h"
#include <WiFi.h>

/**
 * Solo WiFi en modo Access Point (y reconexión STA en loop).
 * No posee el WebServer; eso lo hace WebServerHost.
 */
class WiFiAP
{
public:
  void init(void);
  void loop(void);

  String wifi_name;
  String wifi_ip;

private:
  const char* ssid     = "ESP32-CAM";
  const char* password = "12345678";

  void setup_wifi(void);
};

#endif
