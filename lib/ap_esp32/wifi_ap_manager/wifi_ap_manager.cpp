#include "wifi_ap_manager.h"

void WiFiAP::setup_wifi(void)
{
  // Serial.println(" Configurando LED en pin: " + String(LED_PIN));

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  // Serial.println(" LED apagado inicialmente");

  if (WiFi.status() == WL_CONNECTED)
  {
    // Conexión WiFi existente
  }

  WiFi.mode(WIFI_AP_STA);

  // IP del servidor: los clientes se conectan a esta IP (ej. http://192.168.4.1)
  IPAddress apIp(192, 168, 4, 1);
  IPAddress apGateway(192, 168, 4, 1);
  IPAddress apSubnet(255, 255, 255, 0);
  WiFi.softAPConfig(apIp, apGateway, apSubnet);

  WiFi.softAP(ssid, password);

  wifi_name = String(ssid);
  wifi_ip   = WiFi.softAPIP().toString();

  // Serial.println(" LED encendido - AP listo!");

  if (WiFi.status() == WL_CONNECTED)
  {
    // Conexión WiFi STA mantenida
  }
}

void WiFiAP::init(void)
{
  // Serial.begin(115200); //Ya se llama en el main
  setup_wifi();
  // Serial.println(" Listo! Ve a: http://" + wifi_ip);
}

void WiFiAP::loop(void)
{
  // Mantener conexión WiFi ap
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 30000)
  {
    lastCheck = millis();

    if (WiFi.status() != WL_CONNECTED)
    {
      // Serial.println(" Conexión WiFi STA perdida, intentando reconectar...");
      WiFi.reconnect();
    }
  }
}
