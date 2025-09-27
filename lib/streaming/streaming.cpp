
#include "streaming.h"
#include <Arduino.h>


void Streaming::setup_camera() 
{
  camera_config_t config; //Se define una variable de tipo camera_config_t y se rellena con los valores de la cámara
  
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

  esp_err_t err = esp_camera_init(&config); //Se inicializa la cámara gracias a la librería esp_camera_init 
  if (err != ESP_OK) 
  {
    Serial.printf(" Error inicializando cámara: 0x%x\n", err);
    return;
  }
  
  // === CONFIGURACIÓN DE ORIENTACIÓN ===
  sensor_t *s = esp_camera_sensor_get(); //Obten el sensor de la cámara y guardalo en la variable s
  if (s != NULL) 
  {
    s->set_vflip(s, 1);    // Voltear verticalmente (arriba/abajo)
    // s->set_hmirror(s, 1);  // Voltear horizontalmente (izquierda/derecha)
    // s->set_quality(s, 12); // Ajustar calidad JPEG
  }
  
  Serial.println("Cámara inicializada correctamente");
}

void Streaming::handle_stream() 
{
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  webServer->sendContent(response);

  lastFrameTime = millis(); // Inicializar timer

  while (true) 
  {
    unsigned long currentTime = millis();
    
    // Verificar si es tiempo de enviar el siguiente frame
    if (currentTime - lastFrameTime >= frameInterval) 
    {
      camera_fb_t *fb = esp_camera_fb_get();
      if (!fb) 
      {
        Serial.println("Error capturando frame");
        break;
      }

      String header = "--frame\r\n";
      header += "Content-Type: image/jpeg\r\n";
      header += "Content-Length: " + String(fb->len) + "\r\n\r\n";
      
      webServer->sendContent(header);
      webServer->sendContent((const char *)fb->buf, fb->len);
      webServer->sendContent("\r\n");
      
      esp_camera_fb_return(fb);
      
      lastFrameTime = currentTime; // Actualizar timer
    }
    
    // Permitir que el ESP32 atienda otras tareas
    yield();
  }
}

void Streaming::handle_capture() 
{
  camera_fb_t *fb = esp_camera_fb_get(); //Captura la foto y la guarda en la variable fb
  if (!fb) 
  {
    webServer->send(500, "text/plain", "Error capturando foto"); //Envía un error si no se captura la foto
    return;
  }

  webServer->sendHeader("Content-Disposition", "inline; filename=capture.jpg");//Muestra la foto directamente y no se la descarga
  webServer->sendHeader("Content-Type", "image/jpeg"); //Sin esta linea el navegador no sabría interpretar si es binario o texto, lo etiqueta como jpeg
  webServer->send(200, "image/jpeg", (const char *)fb->buf);
  
  esp_camera_fb_return(fb); //Libera la memoria de la foto
  // Serial.println(" Foto capturada y enviada");
}

void Streaming::init(WebServer* server)
{
  webServer = server;
  setup_camera();
  
  // Configurar rutas del servidor web
  webServer->on("/stream", [this]() { this->handle_stream(); });
  webServer->on("/capture", [this]() { this->handle_capture(); });
  //Aunque pongamos estos endpoints y sea diferentes al punto de acceso principal, si te vasal handle roo veras como 
  //estos endpoints se ejecutan y se muestran en el navegador en concreto lo puedes ver en la linea 32 de la librería wifi_ap.cpp


  Serial.println(" Streaming configurado");
}

void Streaming::loop() 
{
  // El streaming se maneja automáticamente cuando se accede a /stream
  // No necesita loop continuo
}