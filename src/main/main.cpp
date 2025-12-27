#include "command_frame/command_frame.h"
#include "command_sender/command_sender.h"
#include "elegoo_smartcar_lib.h"

#include <Arduino.h>

CommandSender commandSender(Serial2); // Serial2 para comunicación con ATmega

void setup()
{
  // Serial para debug/monitor
  Serial.begin(115200);

  // Serial2 para comunicación con ATmega (pines 40 TX, 3 RX)
  Serial2.begin(9600, SERIAL_8N1, UART2_RX, UART2_TX);

  delay(6000);
  Serial.println("ESP32-S3 iniciado. Comunicación con ATmega por Serial2");
}

void loop()
{
  // Enviar comando de vehículo hacia adelante
  // El ATmega recibirá: {"type":"command","actuators":{"car":{"action":"forward","speed":20}}}
  commandSender.sendCarCommand(CarAction::BACKWARD, 20);
  delay(2000);
}