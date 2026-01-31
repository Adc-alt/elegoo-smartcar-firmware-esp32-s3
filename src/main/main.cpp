// #include "../ap_esp32/ap_esp32.h"
#include "../streaming/streaming.h"
#include "wifi_ap/wifi_ap.h"

#include <Arduino.h>

// CameraStreaming_AP ap_esp32;
Streaming streaming;
WiFiAP wifi_ap;

void setup()
{
  Serial.begin(115200);
  Serial.println("Iniciando...");

  // ap_esp32.init();
  streaming.init(&wifi_ap.server);
  wifi_ap.init();
}

void loop()
{
  //   ap_esp32.loop();
  wifi_ap.loop();
}