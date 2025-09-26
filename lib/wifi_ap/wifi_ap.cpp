#include "wifi_ap.h"
#include "esp_camera.h"
#include "../elegoo_smartcar_lib.h"

void WiFiAP::setup_camera() {
  camera_config_t config;
  
  // Configuración del LEDC (LED Control) para el reloj de la cámara
  config.ledc_channel = LEDC_CHANNEL_0;  // Canal 0 para el reloj
  config.ledc_timer = LEDC_TIMER_0;       // Timer 0 para generar el reloj
  
  // === PINES DE DATOS PARALELOS (8 bits) ===
  // Estos pines transmiten los datos de imagen pixel por pixel
  config.pin_d0 = Y2_GPIO_NUM;  // Bit 0 de datos (LSB - bit menos significativo)
  config.pin_d1 = Y3_GPIO_NUM;  // Bit 1 de datos
  config.pin_d2 = Y4_GPIO_NUM;  // Bit 2 de datos
  config.pin_d3 = Y5_GPIO_NUM;  // Bit 3 de datos
  config.pin_d4 = Y6_GPIO_NUM;  // Bit 4 de datos
  config.pin_d5 = Y7_GPIO_NUM;  // Bit 5 de datos
  config.pin_d6 = Y8_GPIO_NUM;  // Bit 6 de datos
  config.pin_d7 = Y9_GPIO_NUM;  // Bit 7 de datos (MSB - bit más significativo)
  
  // === PINES DE CONTROL DE TIMING ===
  config.pin_xclk = XCLK_GPIO_NUM;    // Reloj maestro externo (10MHz) - sincroniza toda la cámara
  config.pin_pclk = PCLK_GPIO_NUM;    // Reloj de píxel - marca cada píxel de datos
  config.pin_vsync = VSYNC_GPIO_NUM;  // Sincronización vertical - indica inicio de cada frame
  config.pin_href = HREF_GPIO_NUM;    // Sincronización horizontal - indica inicio de cada línea
  
  // === PINES DE COMUNICACIÓN I2C/SCCB ===
  // Para configurar parámetros de la cámara (resolución, brillo, contraste, etc.)
  config.pin_sccb_sda = SIOD_GPIO_NUM;  // Datos SCCB (Serial Camera Control Bus)
  config.pin_sccb_scl = SIOC_GPIO_NUM;  // Reloj SCCB
  
  // === PINES DE CONTROL DE POTENCIA ===
  config.pin_pwdn = PWDN_GPIO_NUM;   // Power Down - activa/desactiva la cámara
  config.pin_reset = RESET_GPIO_NUM; // Reset - reinicia la cámara
  
  // === CONFIGURACIÓN DE LA CÁMARA ===
  config.xclk_freq_hz = 10000000;        // Frecuencia del reloj maestro (10MHz)
  config.pixel_format = PIXFORMAT_JPEG;  // Formato de salida: JPEG comprimido
  config.frame_size = FRAMESIZE_SVGA;    // Resolución: 800x600 píxeles
  config.jpeg_quality = 12;              // Calidad JPEG (0-63, menor = mejor calidad)
  config.fb_count = 1;                   // Número de frame buffers

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("❌ Error inicializando cámara: 0x%x\n", err);
    return;
  }
  
  Serial.println("✅ Cámara inicializada correctamente");
}

void WiFiAP::setup_wifi(void)
{
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  
  wifi_name = String(ssid);
  wifi_ip = WiFi.softAPIP().toString();
  
  Serial.println("📡 WiFi: " + wifi_name);
  Serial.println("🔗 IP: " + wifi_ip);
}

void WiFiAP::handle_root(void)
{
  String html = "<!DOCTYPE html><html><head><title>ESP32 Camera Stream</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { font-family: Arial; text-align: center; margin: 20px; }";
  html += "h1 { color: #333; }";
  html += "img { max-width: 100%; height: auto; border: 2px solid #ddd; }";
  html += "button { background: #4CAF50; color: white; padding: 10px 20px; border: none; border-radius: 5px; margin: 10px; }";
  html += "button:hover { background: #45a049; }";
  html += "</style></head><body>";
  
  html += "<h1>📷 ESP32 Camera Stream</h1>";
  html += "<p>WiFi: " + wifi_name + " | IP: " + wifi_ip + "</p>";
  
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

// void CameraStreaming_AP::handle_stream(void)
// {
//   String response = "HTTP/1.1 200 OK\r\n";
//   response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
//   server.sendContent(response);

//   while (true) {
//     camera_fb_t *fb = esp_camera_fb_get();
//     if (!fb) break;

//     String header = "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: " + String(fb->len) + "\r\n\r\n";
//     server.sendContent(header);
//     server.sendContent((const char *)fb->buf, fb->len);
//     server.sendContent("\r\n");
    
//     esp_camera_fb_return(fb);
//     delay(100);
//   }
// }

// void CameraStreaming_AP::handle_capture(void)
// {
//   camera_fb_t *fb = esp_camera_fb_get();
//   if (!fb) {
//     server.send(500, "text/plain", "Error");
//     return;
//   }
  
//   server.sendHeader("Content-Type", "image/jpeg");
//   server.send(200, "image/jpeg", (const char *)fb->buf);
//   esp_camera_fb_return(fb);
// }

void WiFiAP::setup_server(void)
{
  server.on("/", [this]() { this->handle_root(); });
  server.begin();
  Serial.println("🌐 Servidor web iniciado");
}

void WiFiAP::init(void)
{
  Serial.begin(115200);
  Serial.println("🚀 ESP32 WiFi Access Point");
  
  setup_camera();
  setup_wifi();
  setup_server();
  
  Serial.println("✅ Listo! Ve a: http://" + wifi_ip);
}

void WiFiAP::loop(void)
{
  server.handleClient();
}
