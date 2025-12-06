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


void setup() 
{
  wifiAP.init();
  streaming.init(&wifiAP.server); //Le estoy pasando el puntero del servidor web para que el streaming pueda usarlo y meter sus endpoints
}

void loop() 
{
  wifiAP.loop();
  streaming.loop();
  
  // Permitir que el ESP32 atienda otras tareas y reduzca consumo
  yield(); // Mejor que delay() - no bloquea completamente
}