
#include "streaming.h"
#include "../elegoo_smartcar_lib.h"

void Streaming::init(WebServer* server) {
  webServer = server;
  
  // Configurar rutas del servidor web
  webServer->on("/stream", [this]() { this->handle_stream(); });
  webServer->on("/capture", [this]() { this->handle_capture(); });
  
  Serial.println("📹 Streaming configurado");
}

void Streaming::handle_stream() {
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  webServer->sendContent(response);

  while (true) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("❌ Error capturando frame");
      break;
    }

    String header = "--frame\r\n";
    header += "Content-Type: image/jpeg\r\n";
    header += "Content-Length: " + String(fb->len) + "\r\n\r\n";
    
    webServer->sendContent(header);
    webServer->sendContent((const char *)fb->buf, fb->len);
    webServer->sendContent("\r\n");
    
    esp_camera_fb_return(fb);
    
    // Pequeña pausa para no sobrecargar
    delay(100);
  }
}

void Streaming::handle_capture() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    webServer->send(500, "text/plain", "Error capturando foto");
    return;
  }

  webServer->sendHeader("Content-Disposition", "inline; filename=capture.jpg");
  webServer->sendHeader("Content-Type", "image/jpeg");
  webServer->send(200, "image/jpeg", (const char *)fb->buf);
  
  esp_camera_fb_return(fb);
  Serial.println("📸 Foto capturada y enviada");
}

void Streaming::loop() {
  // El streaming se maneja automáticamente cuando se accede a /stream
  // No necesita loop continuo
}