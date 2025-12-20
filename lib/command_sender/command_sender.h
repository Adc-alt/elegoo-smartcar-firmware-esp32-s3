// command_sender.h
#pragma once
#include "../command_frame/command_frame.h"

#include <Arduino.h>
#include <ArduinoJson.h>

class CommandSender
{
public:
  explicit CommandSender(Stream& out);

  // Enviar comando completo
  void send(const CommandFrame& frame);

  // Métodos helper para enviar comandos individuales
  void sendMotorCommand(bool isLeft, MotorAction action, uint8_t speed);
  void sendServoCommand(uint8_t angle);
  void sendLedCommand(LedColor color);

private:
  Stream& out;
  const char* type = "command";

  const char* motorActionToString(MotorAction action);
  const char* ledColorToString(LedColor color);
};