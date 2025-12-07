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
unsigned long lastSentTime    = 0;  // ← Variable global para timing
int lastButtonValue           = -1; // Valor anterior del botón para detectar cambios

// Declaramos los prototipos de las funciones
void sendMessage(JsonDocument& doc);
void sendCommandImmediate(JsonDocument& doc);
void readMessage();
void checkTimeout();
void processMessage();
void buildGetRequest(const char* target);

// Declaramos la funcion setup
void setup()
{
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, UART2_RX, UART2_TX);
  delay(1500);

  buildGetRequest("button1");
  Serial.println("ESP32 listo");
}

void loop()
{
  sendMessage(docRequest);
  readMessage();
  processMessage(); // Procesar mensajes recibidos
}

void buildGetRequest(const char* target)
{
  docRequest.clear();
  docRequest["type"]   = "get";
  docRequest["target"] = target;
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
  // No mostrar debug del throttle para docRequest (es normal que se bloquee)
}

// Función para enviar comandos inmediatamente sin throttle
void sendCommandImmediate(JsonDocument& doc)
{
  serializeJson(doc, Serial2);
  Serial2.write('\n');
  Serial2.flush(); // Asegurar que se envía inmediatamente
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
  // Si no hay nada, salimos
  if (docReceive.isNull())
    return;

  // type debe existir y ser string
  if (!docReceive["type"].is<const char*>())
  {
    docReceive.clear();
    return;
  }
  const char* type = docReceive["type"];

  // Solo nos interesan las respuestas del ATmega
  if (strcmp(type, "resp") != 0)
  {
    docReceive.clear();
    return;
  }

  // target debe existir y ser string
  if (!docReceive["target"].is<const char*>())
  {
    docReceive.clear();
    return;
  }
  const char* target = docReceive["target"];

  // Solo procesamos respuestas sobre button1
  if (strcmp(target, "button1") != 0)
  {
    docReceive.clear();
    return;
  }

  // value debe existir y ser numérico (0 / 1)
  if (!docReceive["value"].is<int>())
  {
    docReceive.clear();
    return;
  }
  int value = docReceive["value"].as<int>(); // 0 = no pulsado, 1 = pulsado

  // DEBUG opcional
  Serial.print("ESP32: button1 = ");
  Serial.println(value);

  // Detectar cambio de estado del botón
  // (para no spamear comandos si se mantiene pulsado)
  Serial.print("ESP32: DEBUG - value=");
  Serial.print(value);
  Serial.print(", lastButtonValue=");
  Serial.println(lastButtonValue);

  if (value != lastButtonValue)
  {
    Serial.println("ESP32: DEBUG - Cambio detectado!");
    lastButtonValue = value;

    // Si el botón acaba de pasar a pulsado (0 -> 1) → encender LED
    if (value == 1)
    {
      docSend.clear();
      docSend["type"]   = "cmd";
      docSend["target"] = "led1";
      docSend["value"]  = 1; // encender LED

      // Mostrar el JSON que se va a enviar
      Serial.print("ESP32: enviando comando -> ");
      serializeJsonPretty(docSend, Serial);
      Serial.println();

      // Enviar inmediatamente sin throttle
      sendCommandImmediate(docSend);

      Serial.println("ESP32: comando enviado por Serial2");
    }
  }

  // Limpiamos para no reprocesar el mismo mensaje
  docReceive.clear();
}
