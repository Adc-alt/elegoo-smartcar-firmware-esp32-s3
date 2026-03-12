#include "serial_comm.h"

#include "inputs/inputs.h"
#include "outputs/outputs.h"

void SerialComm::initializeJsons()
{
  // Inicializar el objeto JSON de recepción (valores por defecto)
  receiveJson["swPressed"]        = false;
  receiveJson["swCount"]          = 0;
  receiveJson["hcsr04DistanceCm"] = 0;
  receiveJson["lineSensorLeft"]   = 0;
  receiveJson["lineSensorMiddle"] = 0;
  receiveJson["lineSensorRight"]  = 0;
  receiveJson["batVoltage"]       = 0;
  receiveJson["mpuAccelX"]        = 0;
  receiveJson["mpuAccelY"]        = 0;
  receiveJson["mpuAccelZ"]        = 0;
  receiveJson["mpuGyroX"]         = 0;
  receiveJson["mpuGyroY"]         = 0;
  receiveJson["mpuGyroZ"]         = 0;
  receiveJson["irRaw"]            = (unsigned long)0; // Valor IR raw (número entero)

  // Inicializar el objeto JSON de envío (claves compactas para caber en buffer 64B del Atmega)
  // Ver lib/serial_comm/SERIAL_JSON_COMPACT_README.md para equivalencias
  sendJson["sA"] = 90;
  sendJson["lC"] = "Y";
  sendJson["Md"] = 6; // IDLE por defecto (orden enum CarMode)

  JsonObject motors = sendJson.to<JsonObject>().createNestedObject("m");
  JsonObject left  = motors.createNestedObject("L");
  JsonObject right = motors.createNestedObject("R");
  left["a"]        = "fS";
  left["s"]        = 0;
  right["a"]       = "fS";
  right["s"]       = 0;
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
    // Serial.print("Error de deserialización JSON: ");
    // Serial.println(error.c_str());
    // Serial.print("Código de error: ");
    // Serial.println(error.code());
    // Serial.print("JSON recibido: ");
    // Serial.println(jsonString);
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
    // Serial.println("Timeout de recepción");
  }
  else if (lastReceiveTime == 0)
  {
    // Si nunca se ha recibido nada, inicializar el tiempo
    lastReceiveTime = currentTime;
  }
}