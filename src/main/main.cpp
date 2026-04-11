#include "ap_esp32/wifi_ap_manager/wifi_ap_manager.h"
#include "atmega_protocol/atmega_protocol.h"
#include "car_actions/car_actions.h"
#include "elegoo_smartcar_lib.h"
#include "inputs/inputs.h"
#include "mode_manager/mode_manager.h"
#include "outputs/outputs.h"
#include "serial_comm/serial_comm.h"
#include "web/streaming/streaming.h"
#include "web/web_server_host/web_server_host.h"

#include <Arduino.h>

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

static void serviceWeb();
static void readInputs();
static void sendOutputs();

void setup()
{
  Serial.begin(115200);
  // Serial.println("Iniciando...");
  Serial2.begin(115200, SERIAL_8N1, UART2_RX, UART2_TX);
  comm.initializeJsons();

  wifiAp.init();
  webHost.init();
  webHost.setCommandCallback([&](const char* action, int speed) { modeManager.onWebCommand(action, speed, millis()); });

  // Video streaming y controles diferenciales solo permitidos en BALL_FOLLOW_MODE.
  streaming.init(webHost.getServer(), [&]() { return modeManager.isStreamingAllowed(); });

  webHost.setDifferentialCallback(
    [&](const char* leftAction, uint8_t leftSpeed, const char* rightAction, uint8_t rightSpeed)
    { modeManager.onDifferential(leftAction, leftSpeed, rightAction, rightSpeed, millis(), outputData); });
}

void loop()
{
  // Servidor web: siempre atender para que 192.168.4.1 responda (/, /ping, /command, /streaming).
  // Los callbacks ya filtran por modo (comandos solo en RC_MODE, stream solo en BALL_FOLLOW_MODE).
  serviceWeb();
  // 1. LEER ENTRADAS
  readInputs();
  // 2. ACTUALIZAR ESTADOS
  modeManager.updateStates(inputData, outputData);
  // 3. ESCRIBIR SALIDAS (cada SerialComm::kSendIntervalMs ms)
  sendOutputs();
}

static void serviceWeb()
{
  webHost.loop();
  streaming.loop();
}

static void readInputs()
{
  // 1. LEER ENTRADAS
  // Comprobar si hay datos disponibles en serial
  if (Serial2.available() > 0)
  {
    if (comm.readJsonBySerial()) // tardo del orden de 12ms en hacer esta lectura
    {
      fillInputFromAtmegaReceiveJson(comm.receiveJson, inputData);
    }
  }

  // Verificar timeout de recepción
  comm.checkTimeout(); // tardo del orden de 3ms en hacer esta lectura
}

static void sendOutputs()
{
  // 3. ESCRIBIR SALIDAS (cada SerialComm::kSendIntervalMs ms)
  unsigned long currentTime = millis();
  if (currentTime - comm.lastSendTime < SerialComm::kSendIntervalMs)
    return;

  fillSendJsonFromOutputs(comm.sendJson, outputData);
  comm.sendJsonBySerial();
  comm.lastSendTime = currentTime;
}
