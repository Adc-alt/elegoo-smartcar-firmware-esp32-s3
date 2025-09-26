/*
 * CÁMARA ESP32 - STREAMING SIMPLE
 * 
 * Código mínimo para:
 * 1. Crear punto de acceso WiFi
 * 2. Streaming de cámara
 * 3. Interfaz web básica
 */

#include "wifi_ap.h"
#include "streaming.h"

// Instancias de las clases
WiFiAP wifiAP;
Streaming streaming;

void setup() {
  wifiAP.init();
  streaming.init(&wifiAP.server);
}

void loop() {
  wifiAP.loop();
  streaming.loop();
}