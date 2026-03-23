#include "streaming.h"

#include <Arduino.h>
#include <stdio.h>

namespace {

const char kStreamPreamble[] = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n"
                               "Cache-Control: no-store, no-cache, must-revalidate\r\n"
                               "Connection: close\r\n"
                               "\r\n";

} // namespace

void Streaming::setup_camera()
{
  cameraReady = false;
  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;

  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  config.pin_xclk  = XCLK_GPIO_NUM;
  config.pin_pclk  = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href  = HREF_GPIO_NUM;

  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn  = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  // VGA + calidad media: menos bytes por frame → menos presión en WiFi/pila que SVGA+quality 2
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size   = FRAMESIZE_VGA;
  config.jpeg_quality = 12;
  config.fb_count     = 2;
  config.fb_location  = CAMERA_FB_IN_PSRAM;
  config.grab_mode    = CAMERA_GRAB_LATEST;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("[streaming] esp_camera_init failed: 0x%x\n", static_cast<unsigned>(err));
    return;
  }

  sensor_t* s = esp_camera_sensor_get();
  if (s != NULL)
  {
    s->set_vflip(s, 1);

    s->set_whitebal(s, 1);
    s->set_awb_gain(s, 1);
    s->set_exposure_ctrl(s, 1);
    s->set_gain_ctrl(s, 1);

    s->set_brightness(s, 0);
    s->set_contrast(s, 0);
    s->set_saturation(s, 0);
    s->set_sharpness(s, 2);
  }

  cameraReady = true;
}

void Streaming::end_stream_session()
{
  streamSessionActive = false;
  if (streamClient.connected())
    streamClient.stop();
}

void Streaming::handle_stream()
{
  if (!webServer)
    return;

  if (allowStream && !allowStream())
  {
    webServer->send(403, "text/plain", "Solo en modo ball follow");
    return;
  }

  if (!cameraReady)
  {
    webServer->send(503, "text/plain", "Camara no disponible");
    return;
  }

  // Un solo visor: cerrar sesión anterior (navegador + OpenCV peleando el mismo endpoint).
  if (streamClient.connected())
  {
    streamClient.stop();
    delay(2);
  }

  streamClient = webServer->client();
  if (!streamClient.connected())
    return;

  // Cabecera MJPEG por el mismo socket; el cuerpo multipart se envía en loop().
  streamClient.print(kStreamPreamble);
  streamSessionActive = true;
  lastFrameTime       = 0;
}

void Streaming::init(WebServer* server, std::function<bool()> allowStreamFn)
{
  webServer   = server;
  allowStream = allowStreamFn;
  setup_camera();

  webServer->on("/streaming", [this]() { this->handle_stream(); });
}

void Streaming::loop()
{
  if (!streamSessionActive)
    return;

  if (!streamClient.connected())
  {
    end_stream_session();
    return;
  }

  unsigned long now = millis();
  if (now - lastFrameTime < frameInterval)
    return;
  lastFrameTime = now;

  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb)
  {
    end_stream_session();
    return;
  }

  char frameHeader[64];
  int  n = snprintf(frameHeader,
                    sizeof(frameHeader),
                    "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n",
                    static_cast<unsigned>(fb->len));
  if (n <= 0 || static_cast<size_t>(n) >= sizeof(frameHeader))
  {
    esp_camera_fb_return(fb);
    end_stream_session();
    return;
  }

  const size_t hdrLen = static_cast<size_t>(n);
  if (streamClient.write(reinterpret_cast<const uint8_t*>(frameHeader), hdrLen) != hdrLen)
  {
    esp_camera_fb_return(fb);
    end_stream_session();
    return;
  }
  if (streamClient.write(fb->buf, fb->len) != fb->len)
  {
    esp_camera_fb_return(fb);
    end_stream_session();
    return;
  }
  if (streamClient.write(reinterpret_cast<const uint8_t*>("\r\n"), 2) != 2)
  {
    esp_camera_fb_return(fb);
    end_stream_session();
    return;
  }

  esp_camera_fb_return(fb);
}
