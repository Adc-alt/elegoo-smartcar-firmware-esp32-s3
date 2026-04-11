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

  String wifiName;
  String wifiIp;

private:
  static constexpr const char* kApSsid     = "ESP32-CAM";
  static constexpr const char* kApPassword = "12345678";

  void setupWifi(void);
};

#endif
