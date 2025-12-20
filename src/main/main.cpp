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

  delay(1000);
  Serial.println("ESP32-S3 iniciado. Comunicación con ATmega por Serial2");
}

void loop()
{
  // // Ejemplo 1: Enviar comando completo
  // CommandFrame frame;
  // frame.motors.leftAction  = MotorAction::FORWARD;
  // frame.motors.leftSpeed   = 20;
  // frame.motors.rightAction = MotorAction::FORWARD;
  // frame.motors.rightSpeed  = 20;
  // commandSender.send(frame);
  // delay(1000);

  // // Ejemplo 2: Enviar solo motor izquierdo
  // commandSender.sendMotorCommand(true, MotorAction::FORWARD, 100);
  // delay(1000);

  // Ejemplo 3: Enviar servo
  commandSender.sendServoCommand(10);
  delay(2000);
  commandSender.sendServoCommand(170);
  delay(2000);
  commandSender.sendServoCommand(90);
  delay(2000);

  // // Ejemplo 4: Enviar LED
  commandSender.sendLedCommand(LedColor::RED);
  // delay(1000);
}