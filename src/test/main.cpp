#include "car_actions/car_actions.h"
#include "elegoo_smartcar_lib.h"
#include "inputs/inputs.h"
#include "mode_manager/mode_manager.h"
#include "outputs/outputs.h"
#include "rc_mode/rc_mode.h"
#include "serial_comm/serial_comm.h"
#include "web/command_api/command_api.h"
#include "web/web_server_host/web_server_host.h"
#include "web/wifi_ap_manager/wifi_ap_manager.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <cstring>

// Instancia del serialComm comunica con Atmega328p
SerialComm comm;

// Instancia del modeManager gestiona los modos
ModeManager modeManager;

// WiFi AP (solo red); servidor web y comandos en WebServerHost
WiFiAP wifiAp;
WebServerHost webHost;

// Estructuras de datos de entrada(telemetria atmega328p) y salida(control atmega328p)
InputData inputData;
OutputData outputData;

// // Timeout para comandos web (igual que IR: tras 400 ms sin nuevo comando se hace freeStop)
// static const unsigned long WEB_COMMAND_TIMEOUT_MS = 400;
// static unsigned long lastWebCommandTime           = 0;
// static bool webCommandActive                      = false;

void updateInputData();

// Convertir acción larga → código compacto (JSON ≤64 bytes para buffer Atmega)
static const char* actionToShort(const char* action)
{
  if (strcmp(action, "forward") == 0) return "fW";
  if (strcmp(action, "backward") == 0) return "bW";
  if (strcmp(action, "turnLeft") == 0) return "tL";
  if (strcmp(action, "turnRight") == 0) return "tR";
  if (strcmp(action, "freeStop") == 0) return "fS";
  if (strcmp(action, "forceStop") == 0) return "fT";
  return "fS";
}

// Convertir color LED largo → 1 letra (ver SERIAL_JSON_COMPACT_README.md)
static const char* ledColorToShort(const String& color)
{
  if (color == "YELLOW") return "Y";
  if (color == "BLUE") return "B";
  if (color == "GREEN") return "G";
  if (color == "PURPLE") return "P";
  if (color == "WHITE") return "W";
  if (color == "SALMON") return "S";
  if (color == "CYAN") return "C";
  return "Y";
}

void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, UART2_RX, UART2_TX);
  comm.initializeJsons();

  // Inicializar valores por defecto de salida
  // CarActions::freeStop(outputData);
  CarActions::setServoAngle(outputData, 90);
  CarActions::setLedColor(outputData, "YELLOW");

  // wifiAp.init();
  // webHost.init();
  // webHost.setCommandCallback(
  //   [&](const char* action, int speed)
  //   {
  //     if (modeManager.getCurrentMode() != CarMode::RC_MODE)
  //       return;
  //     CommandAPI::execute(action, speed, outputData);
  //     modeManager.getRcModeInstance().onWebCommandReceived(action, millis());
  //   });

  // delay(6000);
}

void loop()
{
  // Servidor
  wifiAp.loop();

  // Servidor web solo en IR_MODE para no bloquear el loop(
  // botón y sensores responden mejor)
  if (modeManager.getCurrentMode() == CarMode::RC_MODE)
    webHost.loop();

  // 1. LEER ENTRADAS
  // Comprobar si hay datos disponibles en serial
  if (Serial2.available() > 0)
  {
    if (comm.readJsonBySerial()) // tardo del orden de 12ms en hacer esta lectura
    {
      updateInputData();
    }
  }

  // Verificar timeout de recepción
  comm.checkTimeout(); // tardo del orden de 3ms en hacer esta lectura

  // 2. ACTUALIZAR ESTADOS
  modeManager.updateStates(inputData, outputData);

  // Timeout comandos web (como IR: tras WEB_COMMAND_TIMEOUT_MS sin nuevo comando → freeStop)
  // if (modeManager.getCurrentMode() == CarMode::IR_MODE && webCommandActive &&
  //     (millis() - lastWebCommandTime >= WEB_COMMAND_TIMEOUT_MS))
  // {
  //   CarActions::freeStop(outputData);
  //   webCommandActive = false;
  // }

  // 3. ESCRIBIR SALIDAS
  // Comprobar si hay que enviar (cada 500ms)
  unsigned long currentTime = millis();
  if (currentTime - comm.lastSendTime >= comm.SEND_INTERVAL)
  {
    // Actualizar sendJson desde outputData (claves compactas, ver SERIAL_JSON_COMPACT_README.md)
    comm.sendJson["sA"]  = outputData.servoAngle;
    comm.sendJson["lC"]  = ledColorToShort(outputData.ledColor);
    comm.sendJson["Md"]  = static_cast<int>(modeManager.getCurrentMode()); // 0-6

    JsonObject motors = comm.sendJson["m"].to<JsonObject>();
    JsonObject left  = motors["L"].to<JsonObject>();
    JsonObject right = motors["R"].to<JsonObject>();
    left["a"]        = actionToShort(outputData.leftAction.c_str());
    left["s"]        = outputData.leftSpeed;
    right["a"]       = actionToShort(outputData.rightAction.c_str());
    right["s"]       = outputData.rightSpeed;

    // Print antes de enviar
    // Serial.print("[ENVIO ATMEGA] Tiempo: ");
    // Serial.print(currentTime);
    // Serial.print("ms - JSON: ");
    // serializeJson(comm.sendJson, Serial); // tardo del orden de 6ms en hacer esta lectura
    // Serial.println();
    comm.sendJsonBySerial();
    comm.lastSendTime = currentTime;
  }
}

void updateInputData()
{
  // Actualizar la estructura de entrada directamente desde receiveJson
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

  // Imprimir los valores deserializados
  // Serial.print("swPressed: ");
  // Serial.print(inputData.swPressed);
  // Serial.print(", swCount: ");
  // Serial.print(inputData.swCount);
  // Serial.print(", hcsr04DistanceCm: ");
  // Serial.print(inputData.hcsr04DistanceCm);
  // Debug: descomenta si necesitas ver sensores (Serial.print ralentiza cada lectura Serial2)
  // Serial.print(", lineSensorLeft: "); Serial.print(inputData.lineSensorLeft);
  // Serial.print(", lineSensorMiddle: "); Serial.print(inputData.lineSensorMiddle);
  // Serial.print(", lineSensorRight: "); Serial.print(inputData.lineSensorRight);
  // Serial.print(", batVoltage: ");
  // Serial.print(inputData.batVoltage);
  // Serial.print(", mpuAccelX: ");
  // Serial.print(inputData.mpuAccelX);
  // Serial.print(", mpuAccelY: ");
  // Serial.print(inputData.mpuAccelY);
  // Serial.print(", mpuAccelZ: ");
  // Serial.print(inputData.mpuAccelZ);
  // Serial.print(", mpuGyroX: ");
  // Serial.print(inputData.mpuGyroX);
  // Serial.print(", mpuGyroY: ");
  // Serial.print(inputData.mpuGyroY);
  // Serial.print(", mpuGyroZ: ");
  // Serial.print(inputData.mpuGyroZ);
  // Serial.print(", irRaw: ");
  // Serial.println(inputData.irRaw);
}