#include "web_server_host.h"

#include "ArduinoJson.h"

void WebServerHost::setup_routes(void)
{
  // Server.on (url, callback) guarda una tabla tiempo: "/led", encenderLed()"
  // 1- Cuando llega una ptecion mira URL
  // 2- Busca la funcion
  // 3- la ejecuta
  server.on("/", [this]() { this->handle_root(); });
  server.on("/ping", [this]() { this->handle_ping(); });
  server.on("/command", HTTP_POST, [this]() { this->handle_command(); });
//   server.on("/stre")
}

void WebServerHost::handle_root(void)
{
  String wifi_name = "ESP32-CAM";
  String wifi_ip   = WiFi.softAPIP().toString();

  String html = "<!DOCTYPE html><html><head><title>ESP32 Camera Stream</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { font-family: Arial; text-align: center; margin: 20px; }";
  html += "h1 { color: #333; }";
  html += ".pad { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 8px; justify-items: center; "
          "align-items: center; max-width: 200px; margin: 20px auto; }";
  html += ".pad button { width: 56px; height: 56px; border: none; border-radius: 8px; color: white; "
          "font-size: 12px; font-weight: bold; cursor: pointer; }";
  html += ".pad .btn-move { background: #4CAF50; } .pad .btn-move:hover { background: #45a049; }";
  html += ".pad .btn-stop { background: #d32f2f; grid-column: 2; } .pad .btn-stop:hover { background: #b71c1c; }";
  html += ".pad .pad-cell { width: 56px; height: 56px; }";
  html += "</style></head><body>";

  html += "<h1>ESP32 Camera Stream</h1>";
  html += "<p>WiFi: " + wifi_name + " | IP: " + wifi_ip + "</p>";

  html += "<div class=\"pad\">";
  html += "<div class=\"pad-cell\"></div>";
  html += "<button type=\"button\" id=\"btn_forward\" class=\"btn-move\">Forward</button>";
  html += "<div class=\"pad-cell\"></div>";
  html += "<button type=\"button\" id=\"btn_left\" class=\"btn-move\">Left</button>";
  html += "<button type=\"button\" id=\"btn_stop\" class=\"btn-stop\">Stop</button>";
  html += "<button type=\"button\" id=\"btn_right\" class=\"btn-move\">Right</button>";
  html += "<div class=\"pad-cell\"></div>";
  html += "<button type=\"button\" id=\"btn_backward\" class=\"btn-move\">Backward</button>";
  html += "<div class=\"pad-cell\"></div>";
  html += "</div>";
  html += "<p id=\"feedback\" style=\"margin: 8px 0; font-weight: bold; min-height: 1.2em;\">Último: -</p>";

  html += "<script>";
  html += "var fb = document.getElementById('feedback');";
  html += "function sendCommand(action, speed) {";
  html += "  fb.textContent = 'Enviando ' + action + '...';";
  html += "  var obj = { action: action, speed: speed || 0 };";
  html += "  fetch('/command', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: "
          "JSON.stringify(obj) })";
  html += "    .then(function(r) { return r.json(); })";
  html += "    .then(function(d) { fb.textContent = 'Enviado: ' + action + ' OK'; })";
  html += "    .catch(function(e) { fb.textContent = 'Error al enviar ' + action; });";
  html += "}";
  html += "document.getElementById('btn_forward').onclick = function() { sendCommand('forward', 70); };";
  html += "document.getElementById('btn_backward').onclick = function() { sendCommand('backward', 70); };";
  html += "document.getElementById('btn_left').onclick = function() { sendCommand('left', 70); };";
  html += "document.getElementById('btn_right').onclick = function() { sendCommand('right', 70); };";
  html += "document.getElementById('btn_stop').onclick = function() { sendCommand('stop', 0); };";
  html += "</script>";

  html += "</body></html>";

  server.send(200, "text/html", html);
}

void WebServerHost::handle_ping(void)
{
  String response = "{";
  response += "\"status\":\"ok\",";
  response += "\"clients\":" + String(WiFi.softAPgetStationNum()) + ",";
  response += "\"wifi_connected\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false");
  response += "}";

  server.send(200, "application/json", response);
}

void WebServerHost::handle_command(void)
{
  Serial.println(">>> POST /command received");

  // server.method() devuelve el metodo de la peticion HTTP
  // GET pedir informacion
  // POST enviar datos
  // PUT/DELETE modificar datos
  if (server.method() != HTTP_POST)
  {
    server.send(405, "application/json", "{\"ok\":false,\"error\":\"Method Not Allowed\"}");
    return;
  }

  // server.arg("plain") comprueba si llego un parametro
  // mira si existe la clave en el mapa de parametros
  String body = server.arg("plain");
  if (body.length() == 0)
  {
    server.send(400, "application/json", "{\"ok\":false,\"error\":\"Empty body\"}");
    return;
  }

  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, body);
  if (err)
  {
    server.send(400, "application/json", "{\"ok\":false,\"error\":\"Invalid JSON\"}");
    return;
  }

  const char* action = doc["action"] | "";
  int speed          = doc["speed"] | 0;

  //   Serial.print("Command: action=");
  //   Serial.print(action);
  //   Serial.print(" speed=");
  //   Serial.println(speed);

  if (commandCallback)
    commandCallback(action, speed);

  server.send(200, "application/json", "{\"ok\":true}");
}

void WebServerHost::setCommandCallback(std::function<void(const char*, int)> cb)
{
  commandCallback = cb;
}

void WebServerHost::init(void)
{
  setup_routes();
  server.begin(); // Método de la clase WebServer que inicia el servidor
                  // oye sistemas, a partir de ahora escucha peticiones http
                  //-Abre socket
                  //-Espienza a escuchar en el puerto
                  // Deja el server activo
}

void WebServerHost::loop(void)
{
  server.handleClient(); // Mira si alguien está llamando y atíendelo
                         // 1-Comprueba si hay un cliente conectado
                         // 2- SI hay datos los lee
                         // 3- analiza la petción HTTP
                         // 4- decide que funcion ejecutar
                         // 5- envia la respuesta
}

WebServer* WebServerHost::getServer(void)
{
  return &server;
}
