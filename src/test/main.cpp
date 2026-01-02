#include "elegoo_smartcar_lib.h"

#include <Arduino.h>
#include <ArduinoJson.h>

// Objetos JSON
JsonDocument sendJson;
JsonDocument receiveJson;

// Variables para el JSON de recepción (comandos)
bool swPressed         = false;
bool swPressedPrevious = false;
int swCount            = 0;
int hcsr04DistanceCm   = 0;
int lineSensorLeft     = 0;
int lineSensorMiddle   = 0;
int lineSensorRight    = 0;
int batVoltage         = 0;
float mpuAccelX        = 0;
float mpuAccelY        = 0;
float mpuAccelZ        = 0;
float mpuGyroX         = 0;
float mpuGyroY         = 0;
float mpuGyroZ         = 0;
String irCommand       = "stop";

// Variables para el JSON de envío (telemetría)
int servoAngle  = 0;
String ledColor = "GREEN";

// Variable para control de tiempo de envío
unsigned long lastSendTime        = 0;
const unsigned long SEND_INTERVAL = 500; // 500ms

// Variables para control de timeout de recepción
unsigned long lastReceiveTime        = 0;
bool timeoutActive                   = false;
const unsigned long TIMEOUT_INTERVAL = 2000; // 2 segundos

// Declaraciones de funciones
void readInput();
void initializeJsons();
void sendJsonBySerial();
void readJsonBySerial();
void checkTimeout();

void setup()
{
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, UART2_RX, UART2_TX);
  initializeJsons();
}

void loop()
{
  // Comprobar si hay que enviar (cada 500ms)
  unsigned long currentTime = millis();
  if (currentTime - lastSendTime >= SEND_INTERVAL)
  {
    sendJsonBySerial();
    lastSendTime = currentTime;
    servoAngle++;
    // Serial.println("swCount: " + String(swCount));
  }
  if (swPressed != swPressedPrevious)
  {
    swPressedPrevious = swPressed;
    // Serial.println("swPressed: " + String(swPressed));
  }

  // Comprobar si hay datos disponibles en serial
  if (Serial2.available() > 0)
  {
    readJsonBySerial();
  }

  // Verificar timeout de recepción
  checkTimeout();

  if (swPressed)
  {
    servoAngle = 90 + swCount * 10;
  }
}

void initializeJsons()
{
  // Inicializar el objeto JSON de recepción
  receiveJson["swPressed"]        = false;
  receiveJson["swCount"]          = 0;
  receiveJson["hcsr04DistanceCm"] = 0;
  receiveJson["lineSensorLeft"]   = 0;
  receiveJson["lineSensorMiddle"] = 0;
  receiveJson["lineSensorRight"]  = 0;
  receiveJson["mpuAccelX"]        = 0;
  receiveJson["mpuAccelY"]        = 0;
  receiveJson["mpuAccelZ"]        = 0;
  receiveJson["mpuGyroX"]         = 0;
  receiveJson["mpuGyroY"]         = 0;
  receiveJson["mpuGyroZ"]         = 0;
  receiveJson["batVoltage"]       = 0;
  receiveJson["irCommand"]        = "stop";

  // Inicializar el objeto JSON de envío
  sendJson["servoAngle"] = 90;
  sendJson["ledColor"]   = ledColor;
}

void sendJsonBySerial()
{
  // Actualizar sendJson a partir de las variables de entrada
  sendJson["servoAngle"] = servoAngle;
  sendJson["ledColor"]   = ledColor;

  // Enviar por serial
  serializeJson(sendJson, Serial2);
  Serial2.println();
}

void readJsonBySerial()
{
  // Leer el JSON recibido
  String jsonString = Serial2.readStringUntil('\n');
  jsonString.trim();

  // Deserializar el JSON recibido
  DeserializationError error = deserializeJson(receiveJson, jsonString);

  if (!error)
  {
    // Actualizar las variables a partir del receiveJson
    swPressed        = receiveJson["swPressed"];
    swCount          = receiveJson["swCount"];
    hcsr04DistanceCm = receiveJson["hcsr04DistanceCm"];
    lineSensorLeft   = receiveJson["lineSensorLeft"];
    lineSensorMiddle = receiveJson["lineSensorMiddle"];
    lineSensorRight  = receiveJson["lineSensorRight"];
    batVoltage       = receiveJson["batVoltage"];
    mpuAccelX        = receiveJson["mpuAccelX"];
    mpuAccelY        = receiveJson["mpuAccelY"];
    mpuAccelZ        = receiveJson["mpuAccelZ"];
    mpuGyroX         = receiveJson["mpuGyroX"];
    mpuGyroY         = receiveJson["mpuGyroY"];
    mpuGyroZ         = receiveJson["mpuGyroZ"];
    irCommand        = receiveJson["irCommand"].as<String>();

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
    Serial.println(lineSensorRight);
    Serial.print(", batVoltage: ");
    Serial.print(mpuAccelX);
    Serial.print(", mpuAccelX: ");
    Serial.print(mpuAccelY);
    Serial.print(", mpuAccelY: ");
    Serial.print(mpuAccelZ);
    Serial.print(", mpuAccelZ: ");
    Serial.print(mpuAccelZ);
    Serial.print(", mpuGyroX: ");
    Serial.print(mpuGyroX);
    Serial.print(", mpuGyroY: ");
    Serial.print(mpuGyroY);
    Serial.print(", mpuGyroZ: ");
    Serial.print(mpuGyroZ);
    Serial.print(", batVoltage: ");
    Serial.print(batVoltage);
    Serial.print(", getIrCommand: ");
    Serial.println(irCommand);

    // Actualizar el tiempo de última recepción exitosa
    lastReceiveTime = millis();
    timeoutActive   = false;
  }
  else
  {
    // Imprimir información del error de deserialización
    Serial.print("Error de deserialización JSON: ");
    Serial.println(error.c_str());
    Serial.print("Código de error: ");
    Serial.println(error.code());
    Serial.print("JSON recibido: ");
    Serial.println(jsonString);
  }
}

void checkTimeout()
{
  unsigned long currentTime = millis();

  // Verificar si ha pasado más de 2 segundos desde la última recepción
  if (lastReceiveTime > 0 && (currentTime - lastReceiveTime >= TIMEOUT_INTERVAL && !timeoutActive))
  {
    timeoutActive = true;
    Serial.println("Timeout de recepción");
  }
  else if (lastReceiveTime == 0)
  {
    // Si nunca se ha recibido nada, inicializar el tiempo
    lastReceiveTime = currentTime;
  }
}
