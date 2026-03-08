#include "streaming.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <stdio.h>

void Streaming::setup_camera()
{
  camera_config_t config; // Se define una variable de tipo camera_config_t y se rellena con los valores de la cámara

  // Configuración del LEDC (LED Control) para el reloj de la cámara
  config.ledc_channel = LEDC_CHANNEL_0; // Canal 0 para el reloj
  config.ledc_timer   = LEDC_TIMER_0;   // Timer 0 para generar el reloj

  // === PINES DE DATOS PARALELOS (8 bits) ===
  // Estos pines transmiten los datos de imagen pixel por pixel
  config.pin_d0 = Y2_GPIO_NUM; // Bit 0 de datos (LSB - bit menos significativo)
  config.pin_d1 = Y3_GPIO_NUM; // Bit 1 de datos
  config.pin_d2 = Y4_GPIO_NUM; // Bit 2 de datos
  config.pin_d3 = Y5_GPIO_NUM; // Bit 3 de datos
  config.pin_d4 = Y6_GPIO_NUM; // Bit 4 de datos
  config.pin_d5 = Y7_GPIO_NUM; // Bit 5 de datos
  config.pin_d6 = Y8_GPIO_NUM; // Bit 6 de datos
  config.pin_d7 = Y9_GPIO_NUM; // Bit 7 de datos (MSB - bit más significativo)

  // === PINES DE CONTROL DE TIMING ===
  config.pin_xclk  = XCLK_GPIO_NUM;  // Reloj maestro externo (10MHz) - sincroniza toda la cámara
  config.pin_pclk  = PCLK_GPIO_NUM;  // Reloj de píxel - marca cada píxel de datos
  config.pin_vsync = VSYNC_GPIO_NUM; // Sincronización vertical - indica inicio de cada frame
  config.pin_href  = HREF_GPIO_NUM;  // Sincronización horizontal - indica inicio de cada línea

  // === PINES DE COMUNICACIÓN I2C/SCCB ===
  // Para configurar parámetros de la cámara (resolución, brillo, contraste, etc.)
  config.pin_sccb_sda = SIOD_GPIO_NUM; // Datos SCCB (Serial Camera Control Bus)
  config.pin_sccb_scl = SIOC_GPIO_NUM; // Reloj SCCB

  // === PINES DE CONTROL DE POTENCIA ===
  config.pin_pwdn  = PWDN_GPIO_NUM;  // Power Down - activa/desactiva la cámara
  config.pin_reset = RESET_GPIO_NUM; // Reset - reinicia la cámara

  // === CONFIGURACIÓN DE LA CÁMARA (optimizada para más FPS / análisis) ===
  config.xclk_freq_hz = 20000000; // Reloj maestro 20MHz
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size   = FRAMESIZE_CIF;  // 400x296: mínimo habitual, JPEG muy pequeños = máximo FPS
  config.jpeg_quality = 28;             // 28: archivos pequeños (0-63; mayor = peor calidad, menos bytes)
  config.fb_count     = 2;             // Doble buffer

  esp_err_t err = esp_camera_init(&config); // Se inicializa la cámara gracias a la librería esp_camera_init
  if (err != ESP_OK)
  {
    // Serial.printf(" Error inicializando cámara: 0x%x\n", err);
    return;
  }

  // === CONFIGURACIÓN DE ORIENTACIÓN ===
  sensor_t* s = esp_camera_sensor_get(); // Obten el sensor de la cámara y guardalo en la variable s
  if (s != NULL)
  {
    s->set_vflip(s, 1); // Voltear verticalmente (arriba/abajo)
    // s->set_hmirror(s, 1);  // Voltear horizontalmente (izquierda/derecha)
    // s->set_quality(s, 12); // Ajustar calidad JPEG
  }

  // Serial.println("Cámara inicializada correctamente");
}

void Streaming::handle_stream()
{
  if (allowStream && !allowStream())
  {
    webServer->send(403, "text/plain", "Solo en modo ball follow");
    return;
  }

  // MJPEG: respuesta raw con sendContent para mantener conexión abierta (como ap_esp32)
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n";
  response += "Cache-Control: no-store, no-cache, must-revalidate\r\n\r\n";
  webServer->sendContent(response);

  // Búfer fijo para el encabezado de cada frame (evita alloc String en cada iteración)
  char frameHeader[64];

  while (webServer->client().connected())
  {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb)
    {
      // Serial.println("Stream: error capturando frame");
      break;
    }

    snprintf(frameHeader, sizeof(frameHeader), "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", (unsigned)fb->len);
    webServer->sendContent(frameHeader);
    webServer->sendContent((const char*)fb->buf, fb->len);
    webServer->sendContent("\r\n");

    esp_camera_fb_return(fb);
    // Sin delay: máximo FPS. El límite es WiFi + tamaño del JPEG (CIF+quality28 = frames pequeños).
  }
}

void Streaming::init(WebServer* server, std::function<bool()> allowStreamFn)
{
  webServer   = server;
  allowStream = allowStreamFn;
  setup_camera();

  webServer->on("/streaming", [this]() { this->handle_stream(); });
  webServer->on("/streaming", HTTP_POST, [this]() { this->handle_differential_command(); });

  // Serial.println(" Streaming configurado");
}

void Streaming::setDifferentialCallback(std::function<void(const char*, uint8_t, const char*, uint8_t)> cb)
{
  differentialCallback = cb;
}

void Streaming::handle_differential_command()
{
  if (webServer->method() != HTTP_POST)
  {
    webServer->send(405, "application/json", "{\"ok\":false,\"error\":\"Method Not Allowed\"}");
    return;
  }

  String body = webServer->arg("plain");
  if (body.length() == 0)
  {
    webServer->send(400, "application/json", "{\"ok\":false,\"error\":\"Empty body\"}");
    return;
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, body);
  if (err)
  {
    webServer->send(400, "application/json", "{\"ok\":false,\"error\":\"Invalid JSON\"}");
    return;
  }

  if (!doc.containsKey("motors"))
  {
    webServer->send(400, "application/json", "{\"ok\":false,\"error\":\"Missing motors\"}");
    return;
  }

  JsonObject motors = doc["motors"].as<JsonObject>();
  if (!motors.containsKey("left") || !motors.containsKey("right"))
  {
    webServer->send(400, "application/json", "{\"ok\":false,\"error\":\"Missing motors.left or motors.right\"}");
    return;
  }

  const char* leftAction  = motors["left"]["action"].as<const char*>();
  const char* rightAction = motors["right"]["action"].as<const char*>();
  if (!leftAction)
    leftAction = "forward";
  if (!rightAction)
    rightAction = "forward";
  uint8_t leftSpeed  = motors["left"]["speed"] | 0;
  uint8_t rightSpeed = motors["right"]["speed"] | 0;

  if (differentialCallback)
    differentialCallback(leftAction, leftSpeed, rightAction, rightSpeed);

  webServer->send(200, "application/json", "{\"ok\":true}");
}

void Streaming::loop()
{
  // El streaming se maneja automáticamente cuando se accede a /stream
  // Sin bucle bloqueante - simple y directo
}
