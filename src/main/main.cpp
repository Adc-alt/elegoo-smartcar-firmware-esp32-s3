#include "../lib/elegoo_smartcar_lib.h"

#include <Arduino.h>
#include <ArduinoJson.h>

// Timeouts
#define TIMEOUT_INTERVAL 250
#define SEND_INTERVAL 100

// Estado de recepción
String buffer;

// Vamos a crear dos mensajes json, el que se envia y el que se recibe(Globales)
JsonDocument docSend;
JsonDocument docReceive;
JsonDocument docRequest;

bool processingMessage        = false; // Bandera que nos servira para identificar el timeout
unsigned long lastMessageTime = 0;
unsigned long lastSentTime    = 0; // ← Variable global para timing

// Declaramos los prototipos de las funciones
void sendMessage(JsonDocument& doc);
void readMessage();
void checkTimeout();
void processMessage();

// Declaramos la funcion setup
void setup()
{
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, UART2_RX, UART2_TX);
  delay(1500);

  // request para leer el HC distancia ultrasonido
  docRequest["H"]  = 1;
  docRequest["N"]  = 21;
  docRequest["D1"] = 2;

  Serial.println("ESP32 listo");
}

void loop()
{
  sendMessage(docRequest);
  readMessage();
  processMessage();
}

void sendMessage(JsonDocument& doc)
{
  unsigned long currentTime = millis();

  if (currentTime - lastSentTime >= SEND_INTERVAL)
  {
    lastSentTime = currentTime;
    serializeJson(doc, Serial2); // Usa el parámetro en lugar de docSend
    Serial2.write('\n');
  }
}

void readMessage()
{
  while (Serial2.available())
  {
    char c = Serial2.read(); // Voy almacenando
    if (!processingMessage)
    {
      processingMessage = true;
    }

    if (c == '\n')
    {
      DeserializationError err = deserializeJson(docReceive, buffer);
      if (err)
      {
        Serial.print("JSON inválido: ");
        Serial.println(err.f_str());
      }
      else
      {
        // Serial.println("JSON recibido y válido:");
        lastMessageTime = millis(); // ← TIMEOUT empieza AQUÍ (después de procesar)
        serializeJsonPretty(docReceive, Serial);
        Serial.write('\n');
      }
      buffer            = "";
      processingMessage = false; // ← Reset cuando termina el mensaje
    }
    else if (c != '\r')
    {
      buffer += c;
    }
  }
}

void checkTimeout() // Actualmente no tiene ninguna utilidad pero en el futuro
                    // puede servir para interrumpir procesos importantes si se
                    // pierde la comunicacion
{
  // EL tiempo se actualiza con el ultimo mensaje
  if ((millis() - lastMessageTime) > TIMEOUT_INTERVAL)
  {
    processingMessage = false;
    Serial.println("TIMEOUT");
  }
}

void processMessage()
{
  if (docReceive.containsKey("distance"))
  {
    int dist = docReceive["distance"];
    Serial.print("Distancia leída: ");
    Serial.println(dist);

    docSend.clear();
    docSend["H"]  = 2;
    docSend["N"]  = 1; // comando motor
    docSend["D1"] = 0; // todos

    if (dist < 10)
    {
      docSend["D2"] = 150; // velocidad
      docSend["D3"] = 2;   // 2 = atrás
      Serial.println("<< ATRÁS motores");
    }
    else
    {
      docSend["D2"] = 0; // parado
      docSend["D3"] = 1; // dirección (no importa si velocidad=0)
      Serial.println("<< QUIETO motores");
    }

    serializeJson(docSend, Serial2);
    Serial2.write('\n');
  }
}