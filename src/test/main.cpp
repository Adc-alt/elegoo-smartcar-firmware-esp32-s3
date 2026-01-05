#include "elegoo_smartcar_lib.h"
#include "serial_comm/serial_comm.h"

#include <Arduino.h>
#include <ArduinoJson.h>

SerialComm comm;

// Variables para el JSON de recepción (comandos)
bool swPressed         = false;
bool swPressedPrevious = false;
int swCount            = 0;
int hcsr04DistanceCm   = 0;
int lineSensorLeft     = 0;
int lineSensorMiddle   = 0;
int lineSensorRight    = 0;
float batVoltage       = 0;
float mpuAccelX        = 0;
float mpuAccelY        = 0;
float mpuAccelZ        = 0;
float mpuGyroX         = 0;
float mpuGyroY         = 0;
float mpuGyroZ         = 0;
String irCommand       = "stop";

// Variables para el JSON de envío (telemetría)
int servoAngle  = 90;
String ledColor = "YELLOW";
String action   = "forward";
uint8_t speed   = 20;

void updateVariables();

void setup()
{
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, UART2_RX, UART2_TX);
  comm.initializeJsons();
}

void loop()
{
  // 1. LEER ENTRADAS
  // Comprobar si hay datos disponibles en serial
  if (Serial2.available() > 0)
  {
    if (comm.readJsonBySerial())
    {
      updateVariables();
    }
  }

  // Verificar timeout de recepción
  comm.checkTimeout();

  // 2. ACTUALIZAR ESTADOS

  // 3. ESCRIBIR SALIDAS
  // Comprobar si hay que enviar (cada 500ms)
  unsigned long currentTime = millis();
  if (currentTime - comm.lastSendTime >= comm.SEND_INTERVAL)
  {
    // Actualizar sendJson antes de enviar
    comm.sendJson["ledColor"]   = ledColor;
    comm.sendJson["servoAngle"] = servoAngle;

    // Actualizar objeto motors anidado
    JsonObject motors = comm.sendJson["motors"].to<JsonObject>();
    motors["action"]  = action;
    motors["speed"]   = speed;

    comm.sendJsonBySerial();
    comm.lastSendTime = currentTime;
  }
}

void updateVariables()
{
  // Actualizar las variables a partir del receiveJson (usar comm.receiveJson)
  swPressed        = comm.receiveJson["swPressed"];
  swCount          = comm.receiveJson["swCount"];
  hcsr04DistanceCm = comm.receiveJson["hcsr04DistanceCm"];
  lineSensorLeft   = comm.receiveJson["lineSensorLeft"];
  lineSensorMiddle = comm.receiveJson["lineSensorMiddle"];
  lineSensorRight  = comm.receiveJson["lineSensorRight"];
  batVoltage       = comm.receiveJson["batVoltage"];
  mpuAccelX        = comm.receiveJson["mpuAccelX"];
  mpuAccelY        = comm.receiveJson["mpuAccelY"];
  mpuAccelZ        = comm.receiveJson["mpuAccelZ"];
  mpuGyroX         = comm.receiveJson["mpuGyroX"];
  mpuGyroY         = comm.receiveJson["mpuGyroY"];
  mpuGyroZ         = comm.receiveJson["mpuGyroZ"];
  irCommand        = comm.receiveJson["irCommand"].as<String>();

  // Imprimir los valores deserializados
  Serial.print("swPressed: ");
  Serial.print(swPressed);
  Serial.print(", swCount: ");
  Serial.print(swCount);
  Serial.print(", hcsr04DistanceCm: ");
  Serial.print(hcsr04DistanceCm);
  Serial.print(", lineSensorLeft: ");
  Serial.print(lineSensorLeft);
  Serial.print(", lineSensorMiddle: ");
  Serial.print(lineSensorMiddle);
  Serial.print(", lineSensorRight: ");
  Serial.print(lineSensorRight);
  Serial.print(", batVoltage: ");
  Serial.print(batVoltage);
  Serial.print(", mpuAccelX: ");
  Serial.print(mpuAccelX);
  Serial.print(", mpuAccelY: ");
  Serial.print(mpuAccelY);
  Serial.print(", mpuAccelZ: ");
  Serial.print(mpuAccelZ);
  Serial.print(", mpuGyroX: ");
  Serial.print(mpuGyroX);
  Serial.print(", mpuGyroY: ");
  Serial.print(mpuGyroY);
  Serial.print(", mpuGyroZ: ");
  Serial.print(mpuGyroZ);
  Serial.print(", irCommand: ");
  Serial.println(irCommand);
}