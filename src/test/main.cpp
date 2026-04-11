#include "ap_esp32/wifi_ap_manager/wifi_ap_manager.h"
#include "ball_follow_mode/ball_follow_mode.h"
#include "car_actions/car_actions.h"
#include "elegoo_smartcar_lib.h"
#include "inputs/inputs.h"
#include "mode_manager/mode_manager.h"
#include "outputs/outputs.h"
#include "atmega_protocol/atmega_protocol.h"
#include "serial_comm/serial_comm.h"
#include "web/streaming/streaming.h"
#include "web/web_server_host/web_server_host.h"

#include <Arduino.h>

// -----------------------------------------------------------------------------
// Firmware de prueba (solo este archivo): aislar Ball Follow sin ciclar modos
// con el switch. Si es true: no se usa updateStates() del ModeManager; solo
// BallFollowMode + POST /motors + GET /streaming. Pon false para comportamiento
// igual que src/main/main.cpp (hay que estar en BALL_FOLLOW_MODE con el switch).
// -----------------------------------------------------------------------------
static constexpr bool kBallFollowTestOnly = true;

// Instancia del serialComm comunica con Atmega328p
SerialComm comm;

// Instancia del modeManager gestiona los modos
ModeManager modeManager;

// WiFi AP (solo red); servidor web y comandos en WebServerHost
WiFiAP wifiAp;
WebServerHost webHost;
Streaming streaming;

// Estructuras de datos de entrada(telemetria atmega328p) y salida(control atmega328p)
InputData inputData;
OutputData outputData;

void updateInputData();
static void sendOutputs();

void setup()
{
  Serial.begin(115200);
  // Serial.println("Iniciando...");
  Serial2.begin(115200, SERIAL_8N1, UART2_RX, UART2_TX);
  comm.initializeJsons();

  // Centro del servo al arrancar (evita que quede girado desde un estado anterior)
  CarActions::setServoAngle(outputData, 90);

  wifiAp.init();
  webHost.init();

  if (kBallFollowTestOnly)
  {
    // Solo prueba Ball Follow: sin comandos RC por web (ModeManager no hace transitionTo; forzamos `Md` en UART).
    modeManager.getBallFollowModeInstance().startMode();
    outputData.modeOrdinal = static_cast<uint8_t>(static_cast<int>(CarMode::BALL_FOLLOW_MODE));
  }
  else
  {
    webHost.setCommandCallback(
      [&](const char* action, int speed) { modeManager.onWebCommand(action, speed, millis()); });
  }

  // Streaming: siempre permitir vídeo en prueba aislada; si no, solo en BALL_FOLLOW_MODE
  streaming.init(webHost.getServer(), [&]()
                   { return kBallFollowTestOnly || modeManager.isStreamingAllowed(); });

  webHost.setDifferentialCallback(
    [&](const char* leftAction, uint8_t leftSpeed, const char* rightAction, uint8_t rightSpeed)
    {
      modeManager.onDifferential(
        leftAction, leftSpeed, rightAction, rightSpeed, millis(), outputData, kBallFollowTestOnly);
    });
}

void loop()
{
  // Servidor WiFi AP
  // wifiAp.loop();

  // Servidor web: /, /ping, /command, POST /motors, y GET /streaming (registrado en Streaming)
  webHost.loop();
  // MJPEG: un frame por vuelta; no bloquea POST /motors ni el resto del loop
  streaming.loop();

  // 1. LEER ENTRADAS
  if (Serial2.available() > 0)
  {
    if (comm.readJsonBySerial()) // tardo del orden de 12ms en hacer esta lectura
    {
      updateInputData();
    }
  }

  comm.checkTimeout();

  // 2. ACTUALIZAR ESTADOS
  if (kBallFollowTestOnly)
  {
    // Aislado: solo lógica Ball Follow (el ModeManager sigue en IDLE internamente)
    modeManager.getBallFollowModeInstance().update(inputData, outputData);
    CarActions::setLedColor(outputData, "CYAN");
  }
  else
  {
    modeManager.updateStates(inputData, outputData);
  }

  // 3. ESCRIBIR SALIDAS (cada SerialComm::kSendIntervalMs ms)
  sendOutputs();
}

void updateInputData()
{
  inputData.swPressed        = comm.receiveJson["swPressed"];
  inputData.swCount          = comm.receiveJson["swCount"];
  inputData.hcsr04DistanceCm = comm.receiveJson["hcsr04DistanceCm"];
  inputData.lineSensorLeft   = comm.receiveJson["lineSensorLeft"];
  inputData.lineSensorMiddle = comm.receiveJson["lineSensorMiddle"];
  inputData.lineSensorRight  = comm.receiveJson["lineSensorRight"];
  inputData.batVoltage       = comm.receiveJson["batVoltage"];
  inputData.mpuAccelX        = comm.receiveJson["mpuAccelX"];
  inputData.mpuAccelY        = comm.receiveJson["mpuAccelY"];
  inputData.mpuAccelZ        = comm.receiveJson["mpuAccelZ"];
  inputData.mpuGyroX         = comm.receiveJson["mpuGyroX"];
  inputData.mpuGyroY         = comm.receiveJson["mpuGyroY"];
  inputData.mpuGyroZ         = comm.receiveJson["mpuGyroZ"];
  inputData.irRaw            = comm.receiveJson["irRaw"];
}

static void sendOutputs()
{
  unsigned long currentTime = millis();
  if (currentTime - comm.lastSendTime < SerialComm::kSendIntervalMs)
    return;

  fillSendJsonFromOutputs(comm.sendJson, outputData);

  comm.sendJsonBySerial();
  comm.lastSendTime = currentTime;
}
