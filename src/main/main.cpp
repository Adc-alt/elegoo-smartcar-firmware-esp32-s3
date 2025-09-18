#include <Arduino.h>
#include <ArduinoJson.h>
#include "../lib/elegoo_smartcar_lib.h"


// Timeouts
#define TIMEOUT_INTERVAL 250
#define INTERVAL 500

// Estado de recepción
String buffer;

//Vamos a crear dos mensajes json, el que se envia y el que se recibe(Globales)
JsonDocument docSend;
JsonDocument docReceive;
JsonDocument docRequest;

bool   processingMessage     = false; // Bandera que nos servira para identificar el timeout 
unsigned long lastMessageTime   = 0;
unsigned long lastSentTime      = 0;  // ← Variable global para timing


//Declaramos los prototipos de las funciones 
void sendMessage(JsonDocument& doc);
void readMessage();
void checkTimeout();
void processMessage();

//Declaramos la funcion setup
void setup() 
{
  Serial.begin(9600);                                
  Serial2.begin(9600, SERIAL_8N1, UART2_RX, UART2_TX); 
  delay(1500);

  // // Inicializar el JSON que se va a enviar
  // docSend["sensor"] = "ESP32";
  // docSend["timestamp"] = millis();
  // docSend["status"] = "ready";

  
  docRequest["H"]=12;
  docRequest["N"]=21;
  

  Serial.println("ESP32 listo para recibir JSON por UART");
}



void loop() 
{
  sendMessage(docRequest);
  readMessage();
  processMessage();
  delay(400);
}



void sendMessage(JsonDocument& doc)
{
  unsigned long currentTime = millis();

  if (currentTime - lastSentTime >= INTERVAL)
  {
    lastSentTime = currentTime;
    serializeJson(doc, Serial2);  // Usa el parámetro en lugar de docSend
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
      } else 
      { 
        // Serial.println("JSON recibido y válido:");
        lastMessageTime = millis();  // ← TIMEOUT empieza AQUÍ (después de procesar)
        serializeJsonPretty(docReceive, Serial);
        Serial.write('\n');
      }
      buffer = "";   
      processingMessage = false;  // ← Reset cuando termina el mensaje
    } else if (c != '\r') 
      {
      buffer += c; 
        }
  }
}

void checkTimeout()// Actualmente no tiene ninguna utilidad pero en el futuro puede servir para interrumpir procesos importantes si se pierde la comunicacion
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
  // Verificar si el mensaje tiene los campos que esperamos
  if (docReceive["H"] == 12 && docReceive["N"] == 21)
  {        
    docSend.clear();
    docSend["H"] = 12;
    docSend["N"] = 21;
    docSend["D1"] = "true";

    // Enviar respuesta por Serial2
    serializeJson(docSend, Serial2);
    Serial2.write('\n'); 
  }
}