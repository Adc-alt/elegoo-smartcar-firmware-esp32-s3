#include "wifi_ap.h"


void WiFiAP::setup_wifi(void)
{
  Serial.println("🔧 Configurando LED en pin: " + String(LED_PIN));
  
  // Configurar LED
  pinMode(LED_PIN, OUTPUT);  // Configurar LED como salida
  digitalWrite(LED_PIN, LOW); // Apagar LED inicialmente
  Serial.println("💡 LED apagado inicialmente");
  
  // Verificar si ya hay una conexión WiFi activa
  if (WiFi.status() == WL_CONNECTED) {
    // Serial.println("📶 Conexión WiFi existente detectada: " + WiFi.SSID());
    // Serial.println("🔗 IP STA: " + WiFi.localIP().toString());
  }
  
  WiFi.mode(WIFI_AP_STA); //Se establece el modo dual: AP + STA (mantiene conexión WiFi existente)
  WiFi.softAP(ssid, password); //Se crea la red con el nombre y contraseña proporcionados
  
  wifi_name = String(ssid); //Se asigna el nombre de la red WiFi
  wifi_ip = WiFi.softAPIP().toString(); //Se asigna la IP de la red WiFi
  
  // Encender LED cuando el AP esté disponible
  digitalWrite(LED_PIN, HIGH);
  Serial.println("✅ LED encendido - AP listo!");
  
  // Serial.println("📡 WiFi AP: " + wifi_name);
  // Serial.println("🔗 IP AP: " + wifi_ip);
  
  // Verificar estado de ambas conexiones
  if (WiFi.status() == WL_CONNECTED) 
  {
    // Serial.println("✅ Conexión WiFi STA mantenida: " + WiFi.SSID());
    // 
  } else {
    // Serial.println("⚠️ No hay conexión WiFi STA activa");
  }
}

void WiFiAP::handle_root(void)
{
  String html = "<!DOCTYPE html><html><head><title>ESP32 Camera Stream</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { font-family: Arial; text-align: center; margin: 20px; }";
  html += "h1 { color: #333; }";
  html += "img { max-width: 100%; height: auto; border: 2px solid #ddd; }";
  // html += "button { background: #4CAF50; color: white; padding: 10px 20px; border: none; border-radius: 5px; margin: 10px; }";
  // html += "button:hover { background: #45a049; }";
  html += "</style></head><body>";
  
  html += "<h1>ESP32 Camera Stream</h1>";
  html += "<p>WiFi: " + wifi_name + " | IP: " + wifi_ip + "</p>";
  
  html += "<div>";
  html += "<img src='/stream' alt='Camera Stream'>";
  html += "</div>";
  
  // html += "<div>";
  // html += "<button onclick='location.reload()'>🔄 Actualizar</button>";
  // html += "<button onclick='window.open(\"/capture\", \"_blank\")'>📸 Capturar Foto</button>";
  // html += "</div>";
  
  // html += "<p><small>Streaming en tiempo real desde ESP32</small></p>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void WiFiAP::handle_ping(void)
{
  // Responder con estado JSON simple
  String response = "{";
  response += "\"status\":\"ok\",";
  response += "\"clients\":" + String(WiFi.softAPgetStationNum()) + ",";
  response += "\"wifi_connected\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false");
  response += "}";
  
  server.send(200, "application/json", response);
}

void WiFiAP::setup_server(void)
{
  server.on("/", [this]() { this->handle_root(); }); //Se define el endPoint "/" y se asigna la función handle_root
  server.on("/ping", [this]() { this->handle_ping(); }); // Endpoint para mantener conexión activa
  server.begin(); //Se inicia el servidor web
  // Serial.println("Servidor web iniciado");
}

void WiFiAP::init(void)
{
  Serial.begin(115200);
  // Serial.println("ESP32 WiFi Access Point");
  
  // setup_camera();
  setup_wifi();   // Configura LED y WiFi, enciende LED cuando AP esté listo
  setup_server();
  
  Serial.println(" Listo! Ve a: http://" + wifi_ip);
}

void WiFiAP::loop(void)
{
  server.handleClient();
  
  // Mantener conexión WiFi STA
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 30000) { // Verificar cada 30 segundos
    lastCheck = millis();
    
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("⚠️ Conexión WiFi STA perdida, intentando reconectar...");
      WiFi.reconnect();
    }
  }
}

