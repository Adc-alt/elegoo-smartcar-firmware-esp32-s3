#include "ap_esp32/wifi_ap_manager/wifi_ap_manager.h"
#include "ball_follow_mode/ball_follow_mode.h"
#include "car_actions/car_actions.h"
#include "elegoo_smartcar_lib.h"
#include "inputs/inputs.h"
#include "mode_manager/mode_manager.h"
#include "outputs/outputs.h"
#include "rc_mode/rc_mode.h"
#include "serial_comm/serial_comm.h"
#include "web/command_api/command_api.h"
#include "web/streaming/streaming.h"
#include "web/web_server_host/web_server_host.h"

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
Streaming streaming;

// Estructuras de datos de entrada(telemetria atmega328p) y salida(control atmega328p)
InputData inputData;
OutputData outputData;

void updateInputData();
static const char* actionToShort(const char* action);
static const char* ledColorToShort(const String& color);

void setup()
{
  Serial.begin(115200);
  // Serial.println("Iniciando...");
  Serial2.begin(115200, SERIAL_8N1, UART2_RX, UART2_TX);
  comm.initializeJsons();

  // Inicializar valores por defecto de salida
  CarActions::freeStop(outputData);
  CarActions::setServoAngle(outputData, 90);
  CarActions::setLedColor(outputData, "YELLOW");

  wifiAp.init();
  webHost.init();
  webHost.setCommandCallback(
    [&](const char* action, int speed)
    {
      if (modeManager.getCurrentMode() != CarMode::RC_MODE)
        return;
      CommandAPI::execute(action, speed, outputData);
      modeManager.getRcModeInstance().onWebCommandReceived(action, millis());
    });

  // streaming.init(webHost.getServer(), []() { return modeManager.getCurrentMode() == CarMode::BALL_FOLLOW_MODE; });
  // streaming.setDifferentialCallback(
  //   [&](const char* leftAction, uint8_t leftSpeed, const char* rightAction, uint8_t rightSpeed)
  //   {
  //     if (modeManager.getCurrentMode() != CarMode::BALL_FOLLOW_MODE)
  //       return;
  //     outputData.leftAction  = leftAction ? leftAction : "forward";
  //     outputData.leftSpeed   = leftSpeed;
  //     outputData.rightAction = rightAction ? rightAction : "forward";
  //     outputData.rightSpeed  = rightSpeed;
  //     modeManager.getBallFollowModeInstance().onDifferentialReceived(millis());
  //   });
}

void loop()
{
  // Servidor WiFi AP
  // wifiAp.loop();

  // Servidor web: siempre atender para que 192.168.4.1 responda (/, /ping, /command, /streaming).
  // Los callbacks ya filtran por modo (comandos solo en RC_MODE, stream solo en BALL_FOLLOW_MODE).
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

  // 3. ESCRIBIR SALIDAS
  // Comprobar si hay que enviar (cada 500ms)
  unsigned long currentTime = millis();
  if (currentTime - comm.lastSendTime >= comm.SEND_INTERVAL)
  {
    // Actualizar sendJson desde outputData (claves compactas, ver SERIAL_JSON_COMPACT_README.md)
    comm.sendJson["sA"] = outputData.servoAngle;
    comm.sendJson["lC"] = ledColorToShort(outputData.ledColor);

<<<<<<< HEAD
    // md = número del modo (orden enum CarMode: IR=0, OBSTACLE=1, FOLLOW=2, LINE=3, RC=4, BALL=5, IDLE=6)
    comm.sendJson["Md"] = static_cast<int>(modeManager.getCurrentMode());

    auto actionToCode = [](const String& a) -> const char* {
      if (a == "forward") return "fW";
      if (a == "backward") return "bW";
      if (a == "turnLeft") return "tL";
      if (a == "turnRight") return "tR";
      if (a == "forceStop") return "fT";
      return "fS"; // freeStop por defecto
    };
=======
>>>>>>> d8fb2749111071840f58408a7ffeb354b672290b
    JsonObject motors = comm.sendJson["m"].to<JsonObject>();
    JsonObject left   = motors["L"].to<JsonObject>();
    JsonObject right  = motors["R"].to<JsonObject>();
    left["a"]         = actionToShort(outputData.leftAction.c_str());
    left["s"]         = outputData.leftSpeed;
    right["a"]        = actionToShort(outputData.rightAction.c_str());
    right["s"]        = outputData.rightSpeed;

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
}

static const char* actionToShort(const char* action)
{
  if (strcmp(action, "forward") == 0)
    return "fW";
  if (strcmp(action, "backward") == 0)
    return "bW";
  if (strcmp(action, "turnLeft") == 0)
    return "tL";
  if (strcmp(action, "turnRight") == 0)
    return "tR";
  if (strcmp(action, "freeStop") == 0)
    return "fS";
  if (strcmp(action, "forceStop") == 0)
    return "fT";
  return "fS";
}

static const char* ledColorToShort(const String& color)
{
  if (color == "YELLOW")
    return "Y";
  if (color == "BLUE")
    return "B";
  if (color == "GREEN")
    return "G";
  if (color == "PURPLE")
    return "P";
  if (color == "WHITE")
    return "W";
  if (color == "SALMON")
    return "S";
  if (color == "CYAN")
    return "C";
  return "Y";
}
