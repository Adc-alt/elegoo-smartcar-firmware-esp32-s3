## Modo RC (`RcMode`)

Modo de control remoto del coche usando WiFi + servidor web + una página HTML con botones.

El objetivo de este documento es explicar **de extremo a extremo** cómo fluye un comando
desde tu móvil/PC hasta que el coche se mueve, pasando por:

- Punto de acceso WiFi (`WiFiAP`)
- Servidor HTTP (`WebServerHost`)
- Página web con botones (HTML + JavaScript)
- Callback de comandos
- Máquina de estados de `RcMode`

### 1. Capa WiFi: punto de acceso (`WiFiAP`)

Archivo relevante: `lib/ap_esp32/wifi_ap_manager/wifi_ap_manager.cpp`

La responsabilidad de esta capa es **únicamente de red**:

- Poner el ESP32 en modo `WIFI_AP_STA`.
- Levantar un **punto de acceso (AP)** propio con una IP fija.
- Mantener la conexión WiFi (reintentos, etc.).

Fragmento clave:

```12:35:lib/ap_esp32/wifi_ap_manager/wifi_ap_manager.cpp
void WiFiAP::setup_wifi(void)
{
  WiFi.mode(WIFI_AP_STA);

  // IP del servidor: los clientes se conectan a esta IP (ej. http://192.168.4.1)
  IPAddress apIp(192, 168, 4, 1);
  IPAddress apGateway(192, 168, 4, 1);
  IPAddress apSubnet(255, 255, 255, 0);
  WiFi.softAPConfig(apIp, apGateway, apSubnet);

  WiFi.softAP(ssid, password);

  wifi_name = String(ssid);
  wifi_ip   = WiFi.softAPIP().toString();
}
```

Puntos importantes:

- El ESP32 crea su propia WiFi (AP) y actúa como **servidor**.
- Los clientes (móvil/PC) se conectan a esa WiFi y usan la IP `192.168.4.1`
  para hablar con el ESP32 vía HTTP.
- Esta capa **no interpreta comandos** ni sabe nada de `RcMode`; solo gestiona la red.

La inicialización se hace en `setup()`:

```37:45:src/main/main.cpp
void setup()
{
  ...
  wifiAp.init();
  ...
}
```

### 2. Servidor HTTP: `WebServerHost`

Archivos: `lib/web/web_server_host/web_server_host.h/.cpp`

`WebServerHost` encapsula un `WebServer` y sus responsabilidades son:

- Arrancar el servidor HTTP (`init` → `server.begin()`).
- Atender peticiones en cada ciclo (`loop` → `server.handleClient()`).
- Registrar rutas:
  - `/` → página HTML de control con botones.
  - `/ping` → estado básico en JSON.
  - `POST /command` → recepción de comandos de movimiento.
- Exponer un `setCommandCallback` para notificar comandos al resto del sistema.

#### 2.1. Rutas registradas

```5:15:lib/web/web_server_host/web_server_host.cpp
void WebServerHost::setup_routes(void)
{
  server.on("/", [this]() { this->handle_root(); });
  server.on("/ping", [this]() { this->handle_ping(); });
  server.on("/command", HTTP_POST, [this]() { this->handle_command(); });
}
```

Y la inicialización en `setup()`:

```37:52:src/main/main.cpp
void setup()
{
  ...
  wifiAp.init();
  webHost.init();
  ...
}
```

En el `loop()` principal se atienden las peticiones:

```68:75:src/main/main.cpp
void loop()
{
  // Servidor web: siempre atender para que 192.168.4.1 responda (/, /ping, /command, /streaming).
  // Los callbacks ya filtran por modo (comandos solo en RC_MODE, stream solo en BALL_FOLLOW_MODE).
  webHost.loop();
  ...
}
```

#### 2.2. Página HTML con botones (`handle_root`)

Cuando entras con el navegador a `http://192.168.4.1/`, el ESP32 responde con una
página HTML generada en `handle_root()`:

```17:73:lib/web/web_server_host/web_server_host.cpp
void WebServerHost::handle_root(void)
{
  ...
  html += "<button type=\"button\" id=\"btn_forward\" class=\"btn-move\">Forward</button>";
  ...
  html += "<button type=\"button\" id=\"btn_left\" class=\"btn-move\">Left</button>";
  html += "<button type=\"button\" id=\"btn_stop\" class=\"btn-stop\">Stop</button>";
  html += "<button type=\"button\" id=\"btn_right\" class=\"btn-move\">Right</button>";
  ...
  html += "<button type=\"button\" id=\"btn_backward\" class=\"btn-move\">Backward</button>";
  ...
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
  ...
  server.send(200, "text/html", html);
}
```

Resumen:

- El navegador muestra una **UI minimalista** con botones: Forward, Backward, Left, Right, Stop.
- Cada botón llama a `sendCommand(action, speed)`.
- `sendCommand` envía un **POST** a `/command` con un **JSON**:

  ```json
  {
    "action": "forward",
    "speed": 70
  }
  ```

#### 2.3. Procesar `POST /command` (`handle_command`)

Cuando llega una petición `POST /command`, se ejecuta:

```86:129:lib/web/web_server_host/web_server_host.cpp
void WebServerHost::handle_command(void)
{
  if (server.method() != HTTP_POST)
  {
    server.send(405, "application/json", "{\"ok\":false,\"error\":\"Method Not Allowed\"}");
    return;
  }

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

  if (commandCallback)
    commandCallback(action, speed);

  server.send(200, "application/json", "{\"ok\":true}");
}
```

Puntos clave:

- Solo se aceptan **métodos POST**.
- El cuerpo de la petición (`body`) debe contener un JSON válido con `action` y `speed`.
- Si hay un `commandCallback` registrado, se llama con `(action, speed)`.
- Después, se responde al cliente con `{"ok":true}`.

### 3. Enganche con `RcMode` en `main.cpp`

En `src/main/main.cpp` se conectan todas las piezas:

- Se inicializan `wifiAp` y `webHost`.
- Se registra un `commandCallback` en `webHost`.
- Ese callback reenvía los comandos al **modo RC** solo si el modo actual es `RC_MODE`.

Fragmento:

```37:52:src/main/main.cpp
void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, UART2_RX, UART2_TX);
  comm.initializeJsons();

  wifiAp.init();
  webHost.init();
  webHost.setCommandCallback(
    [&](const char* action, int speed)
    {
      if (modeManager.getCurrentMode() != CarMode::RC_MODE)
        return;
      modeManager.getRcModeInstance().onWebCommandReceived(action, speed, millis());
    });
}
```

Explicación:

- `webHost.setCommandCallback(...)` registra una lambda que se ejecutará cada vez que
  `handle_command()` reciba un comando válido.
- Primero se comprueba el modo actual:
  - Si el coche **no** está en `CarMode::RC_MODE`, se ignora el comando.
- Si sí está en `RC_MODE`, se llama a:

  ```cpp
  modeManager.getRcModeInstance().onWebCommandReceived(action, speed, millis());
  ```

- Es decir: **el servidor HTTP traduce la petición web a una llamada directa al modo RC**.

### 4. Lógica interna de `RcMode`

Archivos: `lib/rc_mode/rc_mode.h` y `lib/rc_mode/rc_mode.cpp`

`RcMode` hereda de `Mode` y representa el **modo de conducción remota**.

#### 4.1. Máquina de estados

```13:20:lib/rc_mode/rc_mode.h
enum class RcModeState
{
  STOPPED,
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT
};
```

La clase almacena:

- `currentState` y `previousState` → estado actual y anterior.
- `lastSpeed` → velocidad (0–255) asociada al último comando.
- `lastWebCommandTime` → `millis()` del último comando recibido.
- `COMMAND_TIMEOUT_MS` → tiempo máximo sin nuevo comando antes de parar (400 ms).
- `webCommandActive` → indica si hay un comando activo reciente.
- `stopFromWeb` → diferencia entre:
  - `true` → se recibió un `"stop"` explícito desde la web → se usa `forceStop`.
  - `false` → se llegó a STOP por timeout → se usa `freeStop`.

```32:40:lib/rc_mode/rc_mode.h
class RcMode : public Mode
{
public:
  RcMode();
  void startMode() override;
  void stopMode(OutputData& outputData) override;
  bool update(const InputData& inputData, OutputData& outputData) override;
  /** Actualiza el estado del modo con el comando recibido desde la web (callback POST /command). */
  void onWebCommandReceived(const char* action, int speed, unsigned long timestamp);

private:
  RcModeState currentState                      = RcModeState::STOPPED;
  RcModeState previousState                      = RcModeState::STOPPED;
  uint8_t lastSpeed                              = 0;
  unsigned long lastWebCommandTime               = 0;
  static const unsigned long COMMAND_TIMEOUT_MS  = 400;
  bool webCommandActive;
  bool stopFromWeb = false; // true = comando "stop" desde web (forceStop), false = timeout (freeStop)
};
```

#### 4.2. Arranque y parada del modo

```7:21:lib/rc_mode/rc_mode.cpp
RcMode::RcMode()
  : webCommandActive(false)
  , lastWebCommandTime(0)
{
}

void RcMode::startMode()
{
  currentState       = RcModeState::STOPPED;
  previousState      = RcModeState::STOPPED;
  webCommandActive   = false;
  lastWebCommandTime = 0;
  lastSpeed          = 0;
  stopFromWeb        = false;
}

void RcMode::stopMode(OutputData& outputData)
{
  currentState       = RcModeState::STOPPED;
  webCommandActive   = false;
  lastWebCommandTime = 0;
  lastSpeed          = 0;
  stopFromWeb        = false;
  CarActions::forceStop(outputData);
  CarActions::setServoAngle(outputData, 90);
}
```

Comportamiento:

- Al entrar en `RcMode`, el coche arranca en estado `STOPPED`, sin comando activo.
- Al salir del modo, se fuerza una parada (`forceStop`) y se centra el servo a 90°.

#### 4.3. Recepción de comandos web (`onWebCommandReceived`)

Esta función es llamada por el callback registrado en `main.cpp` cuando se recibe
un `POST /command` válido:

```94:125:lib/rc_mode/rc_mode.cpp
void RcMode::onWebCommandReceived(const char* action, int speed, unsigned long timestamp)
{
  lastWebCommandTime = timestamp;

  if (speed < 0)
    lastSpeed = 0;
  else if (speed > 255)
    lastSpeed = 255;
  else
    lastSpeed = static_cast<uint8_t>(speed);

  if (action == nullptr || strcmp(action, "stop") == 0)
  {
    currentState     = RcModeState::STOPPED;
    webCommandActive = false;
    stopFromWeb      = true; // comando "stop" desde web → forceStop
    return;
  }

  if (strcmp(action, "forward") == 0)
    currentState = RcModeState::FORWARD;
  else if (strcmp(action, "backward") == 0)
    currentState = RcModeState::BACKWARD;
  else if (strcmp(action, "left") == 0)
    currentState = RcModeState::LEFT;
  else if (strcmp(action, "right") == 0)
    currentState = RcModeState::RIGHT;
  else
    return; // acción desconocida, no cambiar estado

  webCommandActive = true;
}
```

Detalles:

- Se actualiza `lastWebCommandTime` con el `timestamp` recibido (normalmente `millis()`).
- Se normaliza el `speed` al rango 0–255 y se guarda en `lastSpeed`.
- Si `action` es `nullptr` o `"stop"`:
  - Se pasa a estado `STOPPED`.
  - `webCommandActive = false;`
  - `stopFromWeb = true;` → distinguir un stop explícito desde la web.
- Si `action` es una de las acciones válidas (`forward`, `backward`, `left`, `right`):
  - Se actualiza `currentState` al valor correspondiente.
  - `webCommandActive = true;`
- Si la acción no se reconoce, se ignora y **no se cambia de estado**.

#### 4.4. Bucle de actualización (`update`)

`RcMode::update` se llama desde el `ModeManager` en cada iteración del bucle principal
para aplicar la lógica del modo a los `OutputData`.

```34:92:lib/rc_mode/rc_mode.cpp
bool RcMode::update(const InputData& inputData, OutputData& outputData)
{
  unsigned long currentTime = millis();

  if (webCommandActive && (currentTime - lastWebCommandTime >= COMMAND_TIMEOUT_MS))
  {
    currentState     = RcModeState::STOPPED;
    webCommandActive = false;
    stopFromWeb      = false; // timeout → freeStop
  }

  if (currentState != previousState)
  {
    switch (currentState)
    {
      case RcModeState::FORWARD:
        Serial.println("RcModeState: FORWARD");
        break;
      case RcModeState::BACKWARD:
        Serial.println("RcModeState: BACKWARD");
        break;
      case RcModeState::LEFT:
        Serial.println("RcModeState: LEFT");
        break;
      case RcModeState::RIGHT:
        Serial.println("RcModeState: RIGHT");
        break;
      case RcModeState::STOPPED:
        Serial.println("RcModeState: STOPPED");
        break;
    }
    previousState = currentState;
  }

  switch (currentState)
  {
    case RcModeState::FORWARD:
      CarActions::forward(outputData, lastSpeed);
      break;
    case RcModeState::BACKWARD:
      CarActions::backward(outputData, lastSpeed);
      break;
    case RcModeState::LEFT:
      CarActions::turnLeft(outputData, lastSpeed);
      break;
    case RcModeState::RIGHT:
      CarActions::turnRight(outputData, lastSpeed);
      break;
    case RcModeState::STOPPED:
    default:
      if (stopFromWeb)
        CarActions::forceStop(outputData);
      else
        CarActions::freeStop(outputData);
      break;
  }

  return webCommandActive;
}
```

Comportamiento paso a paso:

1. **Timeout de comando web**
   - Si hay un comando activo (`webCommandActive == true`) y han pasado más de
     `COMMAND_TIMEOUT_MS` milisegundos desde `lastWebCommandTime`:
     - Se pasa a estado `STOPPED`.
     - Se desactiva `webCommandActive`.
     - `stopFromWeb = false;` → indica que la parada ha sido por **timeout**, no por `"stop"`.

2. **Cambio de estado**
   - Si `currentState` ha cambiado respecto a `previousState`, se imprime por `Serial`
     el nuevo estado (`FORWARD`, `BACKWARD`, `LEFT`, `RIGHT`, `STOPPED`), y se actualiza
     `previousState`.

3. **Aplicar acción al coche**
   - Según `currentState`:
     - `FORWARD` → `CarActions::forward(outputData, lastSpeed);`
     - `BACKWARD` → `CarActions::backward(outputData, lastSpeed);`
     - `LEFT` → `CarActions::turnLeft(outputData, lastSpeed);`
     - `RIGHT` → `CarActions::turnRight(outputData, lastSpeed);`
     - `STOPPED` (o cualquier otro valor por defecto):
       - Si `stopFromWeb == true` → `CarActions::forceStop(outputData);`
       - Si `stopFromWeb == false` → `CarActions::freeStop(outputData);`

4. **Valor de retorno**
   - Devuelve `webCommandActive`.
   - Esto permite saber desde fuera si hay un comando web activo reciente (por si
     se quiere tomar decisiones adicionales en el `ModeManager` o en la UI).

### 5. Resumen del flujo completo de un comando

1. **Usuario pulsa un botón** (por ej. Forward) en la página servida en `/`:
   - El navegador llama a `sendCommand('forward', 70);`.
2. **El navegador envía un `POST /command`** a `http://192.168.4.1/command`:
   - Cuerpo JSON: `{"action": "forward", "speed": 70}`.
3. **`WebServerHost::handle_command` procesa la petición**:
   - Verifica método y JSON.
   - Extrae `action` y `speed`.
   - Llama a `commandCallback(action, speed)`.
4. **En `main.cpp`, el callback redirige al modo RC**:
   - Comprueba que el modo actual es `RC_MODE`.
   - Llama a `modeManager.getRcModeInstance().onWebCommandReceived(action, speed, millis());`.
5. **`RcMode::onWebCommandReceived` actualiza el estado interno**:
   - Guarda `lastWebCommandTime` y `lastSpeed`.
   - Cambia `currentState` a `FORWARD`/`BACKWARD`/`LEFT`/`RIGHT` o `STOPPED`.
   - Marca `webCommandActive` y `stopFromWeb` según corresponda.
6. **En el bucle principal, `modeManager.updateStates` llama a `RcMode::update`**:
   - Gestiona timeout de comandos.
   - Aplica la acción adecuada con `CarActions` (`forward`, `backward`, `turnLeft`,
     `turnRight`, `forceStop`, `freeStop`).
7. **`sendOutputs()` envía las salidas al Atmega328p**:
   - `outputData` se traduce a JSON compacto por `SerialComm` y se envía por `Serial2`.

De esta forma, un simple click en el navegador recorre toda la cadena:

**Botón HTML → JavaScript (fetch POST) → WebServerHost (/command) → callback de comandos → RcMode (máquina de estados) → CarActions → Atmega328p → movimiento del coche.**

