#include "car_actions/car_actions.h"
#include "elegoo_smartcar_lib.h"
#include "inputs/inputs.h"
#include "mode_manager/mode_manager.h"
#include "outputs/outputs.h"
#include "serial_comm/serial_comm.h"

#include <Arduino.h>
#include <ArduinoJson.h>

// Instancia del serialComm comunica con Atmega328p
SerialComm comm;

// Instancia del modeManager gestiona los modos
ModeManager modeManager;

// Estructuras de datos de entrada(telemetria atmega328p) y salida(control atmega328p)
InputData inputData;
OutputData outputData;

void updateInputData();

void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, UART2_RX, UART2_TX);
  Serial2.setTimeout(30); // Evitar bloqueos de ~1s en readStringUntil('\n') si no llega \n a tiempo
  comm.initializeJsons();

  // Inicializar valores por defecto de salida
  CarActions::freeStop(outputData);
  CarActions::setServoAngle(outputData, 90);
  CarActions::setLedColor(outputData, "YELLOW");
  delay(6000);
}

void loop()
{
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
  // Aquí irá el modeManager cuando lo implementes
  modeManager.updateStates(inputData, outputData);

  // 3. ESCRIBIR SALIDAS
  // Comprobar si hay que enviar (cada SEND_INTERVAL ms)
  unsigned long currentTime = millis();
  if (currentTime - comm.lastSendTime >= comm.SEND_INTERVAL)
  {
    // Actualizar sendJson desde outputData
    comm.sendJson["ledColor"]   = outputData.ledColor;
    comm.sendJson["servoAngle"] = outputData.servoAngle;

    // Actualizar objeto motors anidado
    JsonObject motors = comm.sendJson["motors"].to<JsonObject>();
    motors["action"]  = outputData.action;
    motors["speed"]   = outputData.speed;

    comm.sendJsonBySerial();
    comm.lastSendTime = currentTime;
  }

  // Dar CPU a USB/FreeRTOS: evita que el loop monopolice y el CDC no vacíe Serial.
  // Sin esto, Serial.println puede bloquear al llenarse el buffer → loop y coche congelados.
  delay(1);
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
  // Serial.print(", lineSensorLeft: ");
  // Serial.print(inputData.lineSensorLeft);
  // Serial.print(", lineSensorMiddle: ");
  // Serial.print(inputData.lineSensorMiddle);
  // Serial.print(", lineSensorRight: ");
  // Serial.print(inputData.lineSensorRight);
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