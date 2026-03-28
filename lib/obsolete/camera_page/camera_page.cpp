#include "camera_page.h"

static const char CAMERA_PAGE_HTML[] = R"(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Camera</title>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <style>
    body { font-family: Arial; text-align: center; margin: 20px; }
    img { max-width: 100%; height: auto; border: 2px solid #ddd; }
    button { background: #4CAF50; color: white; padding: 10px 20px; border: none; border-radius: 5px; margin: 10px; }
  </style>
</head>
<body>
  <h1>ESP32 Camera</h1>
  <img src='/stream' alt='Camera Stream'>
  <br>
  <button onclick='location.reload()'>Actualizar</button>
  <button onclick='window.open("/capture", "_blank")'>📸 Foto</button>
</body>
</html>
)";

String getCameraPageHtml(void)
{
  return String(CAMERA_PAGE_HTML);
}
