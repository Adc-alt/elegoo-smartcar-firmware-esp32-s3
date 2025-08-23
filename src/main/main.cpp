#include <Arduino.h>
#include <ArduinoJson.h>

// UART2 pins (ajusta si hace falta)
#define UART2_TX 40
#define UART2_RX 3

// Timeouts
#define TIMEOUT_INTERVAL 250
// Estado de recepción
String buffer;
bool   receiving     = false;
unsigned long lastMessageTime   = 0;

JsonDocument doc;


void readMessage();
void checkTimeout();

void setup() {
  Serial.begin(115200);                                
  Serial2.begin(9600, SERIAL_8N1, UART2_RX, UART2_TX); 
  Serial.println("ESP32 listo para recibir JSON por UART");
}



void loop() 
{
  readMessage();
  checkTimeout();
}

void readMessage() 
{
  while (Serial2.available()) {
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


