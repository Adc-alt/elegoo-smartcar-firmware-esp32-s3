#include "serial_comm.h"

void SerialComm::initializeJsons()
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
  sendJson["servoAngle"] = 120;
  sendJson["ledColor"]   = "GREEN";

  // Inicializar objeto motors anidado
  JsonObject motors = sendJson.createNestedObject("motors");
  motors["action"]  = "free_stop";
  motors["speed"]   = 20;
}

void SerialComm::sendJsonBySerial()
{
  // Enviar el JSON (los valores ya están actualizados desde fuera)
  serializeJson(sendJson, Serial2);
  Serial2.println();
}

bool SerialComm::readJsonBySerial()
{
  // Verificar que hay datos disponibles
  if (Serial2.available() <= 0)
  {
    return false;
  }

  // Leer el JSON recibido
  String jsonString = Serial2.readStringUntil('\n');
  jsonString.trim();

  // Si el string está vacío, no hay datos válidos
  if (jsonString.length() == 0)
  {
    return false;
  }

  // Deserializar el JSON recibido
  DeserializationError error = deserializeJson(receiveJson, jsonString);

  if (!error)
  {
    // Actualizar el tiempo de última recepción exitosa
    lastReceiveTime = millis();
    timeoutActive   = false;
    return true;
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
    return false;
  }
}

void SerialComm::checkTimeout()
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