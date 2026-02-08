// #include "../ap_esp32/ap_esp32.h"
#include "web/streaming/streaming.h"
#include "web/web_server_host/web_server_host.h"
#include "web/wifi_ap_manager/wifi_ap_manager.h"

#include <Arduino.h>

Streaming streaming;
WiFiAP wifi_ap;
WebServerHost webHost;

void setup()
{
  Serial.begin(115200);
  Serial.println("Iniciando...");

  wifi_ap.init();
  webHost.init();
  streaming.init(webHost.getServer());
}

void loop()
{
  wifi_ap.loop();
  webHost.loop();
}