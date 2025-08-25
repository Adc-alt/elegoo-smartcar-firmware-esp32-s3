#include <Arduino.h>
#include <ArduinoJson.h>
#include "../lib/elegoo_smartcar_lib.h"


// Timeouts
#define TIMEOUT_INTERVAL 250
const unsigned long PERIOD_MS = 50; // 
// Estado de recepción
String buffer;
bool   receiving     = false;
unsigned long lastMessageTime   = 0;
unsigned long lastSentTime      = 0;

JsonDocument doc;


void readMessage();
void checkTimeout();
void sendMessage(JsonDocument &doc, unsigned long interval);

void setup() 
{
  Serial.begin(115200);                                
  Serial2.begin(9600, SERIAL_8N1, UART2_RX, UART2_TX); 
  Serial.println("ESP32 listo para recibir JSON por UART");
  // Inicializar valores del JSON
  doc["temp"] = 23.5;
  doc["hum"]  = 60;
}



void loop() 
{
  sendMessage(doc,PERIOD_MS);
  // readMessage();
  // checkTimeout();
}

void readMessage() 
{
  while (Serial2.available()) 
  {
    char c = Serial2.read();

    
    if (!receiving) 
    {
      receiving = true;
    }

    if (c == '\n') {      
      DeserializationError err = deserializeJson(doc, buffer);
      if (err) {
        Serial.print("JSON inválido: ");
        Serial.println(err.f_str());
      } else {
        Serial.println("JSON recibido y válido:");
        lastMessageTime=millis();
        serializeJsonPretty(doc, Serial);
        Serial.println();
      }
      buffer = "";      
    } else if (c != '\r') {
      buffer += c; 
    }
  }
}

void checkTimeout()
{  
  if (receiving && (millis() - lastMessageTime > TIMEOUT_INTERVAL)) {
    Serial.println("TIMEOUT");
    receiving = false;
  }
}


void sendMessage(JsonDocument &doc, unsigned long interval)
{
  unsigned long currentTime = millis();

  if (currentTime - lastSentTime >= interval)  // ← Usar variable global
  {
    lastSentTime = currentTime;
    serializeJson(doc, Serial2);
    Serial2.println();
  }
  Serial.println("Mensaje enviado");  
}

