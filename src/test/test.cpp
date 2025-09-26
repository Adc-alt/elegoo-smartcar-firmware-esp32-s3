/*
 * CÁMARA ESP32 - STREAMING SIMPLE
 * 
 * Código mínimo para:
 * 1. Crear punto de acceso WiFi
 * 2. Streaming de cámara
 * 3. Interfaz web básica
 */

 #include "esp_camera.h"
 #include <WiFi.h>
 #include <WebServer.h>
 #include <esp_http_server.h>
 
 // ==========================================
 // CONFIGURACIÓN DE CÁMARA
 // ==========================================
 #define CAMERA_MODEL_M5STACK_WIDE
 
 // Pines de la cámara (ajusta según tu placa)
 #define PWDN_GPIO_NUM     32
 #define RESET_GPIO_NUM    -1
 #define XCLK_GPIO_NUM      0
 #define SIOD_GPIO_NUM     26
 #define SIOC_GPIO_NUM     27
 #define Y9_GPIO_NUM       35
 #define Y8_GPIO_NUM       34
 #define Y7_GPIO_NUM       39
 #define Y6_GPIO_NUM       36
 #define Y5_GPIO_NUM       21
 #define Y4_GPIO_NUM       19
 #define Y3_GPIO_NUM       18
 #define Y2_GPIO_NUM        5
 #define VSYNC_GPIO_NUM    25
 #define HREF_GPIO_NUM     23
 #define PCLK_GPIO_NUM     22
 
 // ==========================================
 // CONFIGURACIÓN WIFI
 // ==========================================
 const char* ssid = "ESP32-CAM";
 const char* password = "12345678";
 
 WebServer server(80);
 
 // ==========================================
 // CONFIGURAR CÁMARA
 // ==========================================
 void configurar_camara() {
   camera_config_t config;
   config.ledc_channel = LEDC_CHANNEL_0;
   config.ledc_timer = LEDC_TIMER_0;
   config.pin_d0 = Y2_GPIO_NUM;
   config.pin_d1 = Y3_GPIO_NUM;
   config.pin_d2 = Y4_GPIO_NUM;
   config.pin_d3 = Y5_GPIO_NUM;
   config.pin_d4 = Y6_GPIO_NUM;
   config.pin_d5 = Y7_GPIO_NUM;
   config.pin_d6 = Y8_GPIO_NUM;
   config.pin_d7 = Y9_GPIO_NUM;
   config.pin_xclk = XCLK_GPIO_NUM;
   config.pin_pclk = PCLK_GPIO_NUM;
   config.pin_vsync = VSYNC_GPIO_NUM;
   config.pin_href = HREF_GPIO_NUM;
   config.pin_sccb_sda = SIOD_GPIO_NUM;
   config.pin_sccb_scl = SIOC_GPIO_NUM;
   config.pin_pwdn = PWDN_GPIO_NUM;
   config.pin_reset = RESET_GPIO_NUM;
   config.xclk_freq_hz = 10000000;
   config.pixel_format = PIXFORMAT_JPEG;
   config.frame_size = FRAMESIZE_SVGA;  // 800x600
   config.jpeg_quality = 12;
   config.fb_count = 1;
 
   // Inicializar cámara
   esp_err_t err = esp_camera_init(&config);
   if (err != ESP_OK) {
     Serial.printf("❌ Error inicializando cámara: 0x%x\n", err);
     return;
   }
   
   Serial.println("✅ Cámara inicializada correctamente");
 }
 
 // ==========================================
 // CONFIGURAR WIFI AP
 // ==========================================
 void configurar_wifi() {
   WiFi.mode(WIFI_AP);
   WiFi.softAP(ssid, password);
   
   Serial.println("📡 Punto de acceso creado:");
   Serial.printf("   SSID: %s\n", ssid);
   Serial.printf("   Password: %s\n", password);
   Serial.printf("   IP: %s\n", WiFi.softAPIP().toString().c_str());
 }
 
 // ==========================================
 // PÁGINA WEB PRINCIPAL
 // ==========================================
 void handle_root() {
   String html = "<!DOCTYPE html><html><head>";
   html += "<title>ESP32 Camera Stream</title>";
   html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
   html += "<style>";
   html += "body { font-family: Arial; text-align: center; margin: 20px; }";
   html += "h1 { color: #333; }";
   html += "img { max-width: 100%; height: auto; border: 2px solid #ddd; }";
   html += "button { background: #4CAF50; color: white; padding: 10px 20px; border: none; border-radius: 5px; margin: 10px; }";
   html += "button:hover { background: #45a049; }";
   html += "</style></head><body>";
   
   html += "<h1>📷 ESP32 Camera Stream</h1>";
   html += "<p>Conectado a: " + String(ssid) + "</p>";
   html += "<p>IP: " + WiFi.softAPIP().toString() + "</p>";
   
   html += "<div>";
   html += "<img src='/stream' alt='Camera Stream'>";
   html += "</div>";
   
   html += "<div>";
   html += "<button onclick='location.reload()'>🔄 Actualizar</button>";
   html += "<button onclick='window.open(\"/capture\", \"_blank\")'>📸 Capturar Foto</button>";
   html += "</div>";
   
   html += "<p><small>Streaming en tiempo real desde ESP32</small></p>";
   html += "</body></html>";
   
   server.send(200, "text/html", html);
 }
 
 // ==========================================
 // STREAMING DE VIDEO
 // ==========================================
 void handle_stream() {
   WiFiClient client = server.client();
   String response = "HTTP/1.1 200 OK\r\n";
   response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
   server.sendContent(response);
 
   while (true) {
     camera_fb_t *fb = esp_camera_fb_get();
     if (!fb) {
       Serial.println("❌ Error capturando frame");
       break;
     }
 
     String header = "--frame\r\n";
     header += "Content-Type: image/jpeg\r\n";
     header += "Content-Length: " + String(fb->len) + "\r\n\r\n";
     
     server.sendContent(header);
     server.sendContent((const char *)fb->buf, fb->len);
     server.sendContent("\r\n");
     
     esp_camera_fb_return(fb);
     
     // Pequeña pausa para no sobrecargar
     delay(100);
   }
 }
 
 // ==========================================
 // CAPTURAR FOTO
 // ==========================================
 void handle_capture() {
   camera_fb_t *fb = esp_camera_fb_get();
   if (!fb) {
     server.send(500, "text/plain", "Error capturando foto");
     return;
   }
 
   server.sendHeader("Content-Disposition", "inline; filename=capture.jpg");
   server.sendHeader("Content-Type", "image/jpeg");
   server.send(200, "image/jpeg", (const char *)fb->buf);
   
   esp_camera_fb_return(fb);
   Serial.println("📸 Foto capturada y enviada");
 }
 
 // ==========================================
 // CONFIGURAR SERVIDOR WEB
 // ==========================================
 void configurar_servidor() {
   server.on("/", handle_root);
   server.on("/stream", handle_stream);
   server.on("/capture", handle_capture);
   
   server.begin();
   Serial.println("🌐 Servidor web iniciado");
   Serial.println("📱 Abre tu navegador y ve a: http://" + WiFi.softAPIP().toString());
 }
 
 // ==========================================
 // SETUP
 // ==========================================
 void setup() {
   Serial.begin(115200);
   Serial.println("\n🚀 === ESP32 CAMERA STREAMING SIMPLE ===");
   
   // Configurar cámara
   configurar_camara();
   
   // Configurar WiFi
   configurar_wifi();
   
   // Configurar servidor web
   configurar_servidor();
   
   Serial.println("✅ Sistema listo!");
   Serial.println("📱 Conecta tu teléfono a la red: " + String(ssid));
   Serial.println("🔑 Password: " + String(password));
   Serial.println("🌐 Ve a: http://" + WiFi.softAPIP().toString());
 }
 
 // ==========================================
 // LOOP
 // ==========================================
 void loop() {
   server.handleClient();
   
   // Mostrar estadísticas cada 10 segundos
   static unsigned long last_stats = 0;
   if (millis() - last_stats > 10000) {
     Serial.printf("📊 Clientes conectados: %d\n", WiFi.softAPgetStationNum());
     last_stats = millis();
   }
 }
 