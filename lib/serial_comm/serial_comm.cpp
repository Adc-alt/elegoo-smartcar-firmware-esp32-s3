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

  // Inicializar el objeto JSON de envío
  sendJson["servoAngle"] = 90;
  sendJson["ledColor"]   = "YELLOW";

  // Inicializar objeto motors anidado
  JsonObject motors = sendJson.createNestedObject("motors");
  motors["action"]  = "free_stop";
  motors["speed"]   = 0;

  _rxLineBuffer = "";
}

void SerialComm::sendJsonBySerial()
{
  // No bloquear en Serial2: si el Atmega no lee, el TX se llena y write bloquea el loop.
  // availableForWrite() existe en HardwareSerial (ESP32). Si no hay espacio, omitir este envío.
#if defined(ARDUINO_ARCH_ESP32)
  if (Serial2.availableForWrite() >= 0 && Serial2.availableForWrite() < 160)
    return; // TX casi lleno: omitir este envío en vez de bloquear
#endif
  serializeJson(sendJson, Serial2);
  Serial2.println();
}

bool SerialComm::readJsonBySerial()
{
  // Lectura NO BLOQUEANTE: solo Serial2.read() con available()>0 (sin readStringUntil).
  // Límite de bytes por llamada: si el Atmega manda mucha basura sin \n, no pasamos
  // toda la iteración en este while (evita “congelar” el loop).
  size_t readCount = 0;
  const size_t maxReadPerCall = 256;

  while (Serial2.available() > 0 && readCount < maxReadPerCall)
  {
    int c = Serial2.read();
    if (c < 0)
      break;
    readCount++;

    if (c == '\n' || c == '\r')
    {
      _rxLineBuffer.trim();
      if (_rxLineBuffer.length() > 0)
      {
        DeserializationError error = deserializeJson(receiveJson, _rxLineBuffer);
        _rxLineBuffer = "";
        if (!error)
        {
          lastReceiveTime = millis();
          timeoutActive   = false;
          return true;
        }
      }
      _rxLineBuffer = "";
    }
    else
    {
      _rxLineBuffer += (char)c;
      if (_rxLineBuffer.length() > RX_LINE_MAX)
        _rxLineBuffer = "";
    }
  }
  return false;
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