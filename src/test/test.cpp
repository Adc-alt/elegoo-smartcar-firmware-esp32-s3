/*
 * CÁMARA ESP32 - STREAMING SIMPLE
 * 
 * Código mínimo para:
 * 1. Crear punto de acceso WiFi
 * 2. Streaming de cámara
 * 3. Interfaz web básica
 */

#include "ap_esp32.h"

// Instancia de la clase
CameraStreaming_AP camera;

void setup() {
  camera.init();
}

void loop() {
  camera.loop();
}