#include "command_sender.h"

#include "../command_frame/command_frame.h"

#include <Arduino.h>
#include <ArduinoJson.h>

void CommandSender::send(const CommandFrame& frame)
{
  StaticJsonDocument<256> doc;

  // Cabecera
  doc["type"] = type;

  // Crear objeto actuators solo si hay algo que enviar
  JsonObject actuators = doc.createNestedObject("actuators");

  // Motores - SOLO si hay acción definida
  if (frame.motors.leftAction != MotorAction::NONE || frame.motors.rightAction != MotorAction::NONE)
  {
    JsonObject motors = actuators.createNestedObject("motors");

    if (frame.motors.leftAction != MotorAction::NONE)
    {
      JsonObject left = motors.createNestedObject("left");
      left["action"]  = motorActionToString(frame.motors.leftAction);
      left["speed"]   = frame.motors.leftSpeed;
    }

    if (frame.motors.rightAction != MotorAction::NONE)
    {
      JsonObject right = motors.createNestedObject("right");
      right["action"]  = motorActionToString(frame.motors.rightAction);
      right["speed"]   = frame.motors.rightSpeed;
    }
  }

  // Servo - SOLO si hay comando
  if (frame.servoHasCommand)
  {
    JsonObject servo = actuators.createNestedObject("servo");
    servo["angle"]   = frame.servoAngle;
  }

  // LED - SOLO si hay comando
  if (frame.ledHasCommand)
  {
    JsonObject led = actuators.createNestedObject("led");
    led["color"]   = ledColorToString(frame.ledColor);
  }
// DEBUG: Solo en modo debug
#ifdef DEBUG_COMMAND_SENDER
  Serial.print(F("[CommandSender] Enviando: "));
  serializeJsonPretty(doc, Serial);
  Serial.println();
#endif
  // Verificar overflow
  if (doc.overflowed())
  {
    out.print(F("ERROR: JSON overflow"));
    out.println();
    return;
  }

  // Enviar por Serial terminado con \n
  serializeJson(doc, out);
  out.println();
}

CommandSender::CommandSender(Stream& out) : out(out)
{
}

void CommandSender::sendMotorCommand(bool isLeft, MotorAction action, uint8_t speed)
{
  CommandFrame frame;
  if (isLeft)
  {
    frame.motors.leftAction = action;
    frame.motors.leftSpeed  = speed;
  }
  else
  {
    frame.motors.rightAction = action;
    frame.motors.rightSpeed  = speed;
  }
  send(frame);
}

void CommandSender::sendServoCommand(uint8_t angle)
{
  CommandFrame frame;
  frame.servoHasCommand = true;
  frame.servoAngle      = angle;
  send(frame);
}

void CommandSender::sendLedCommand(LedColor color)
{
  CommandFrame frame;
  frame.ledHasCommand = true;
  frame.ledColor      = color;
  send(frame);
}

const char* CommandSender::motorActionToString(MotorAction action)
{
  switch (action)
  {
    case MotorAction::FORWARD:
      return "forward";
    case MotorAction::REVERSE:
      return "reverse";
    case MotorAction::FORCE_STOP:
      return "force_stop";
    case MotorAction::FREE_STOP:
      return "free_stop";
    case MotorAction::NONE:
    default:
      return "none";
  }
}

const char* CommandSender::ledColorToString(LedColor color)
{
  switch (color)
  {
    case LedColor::BLACK:
      return "black";
    case LedColor::BLUE:
      return "blue";
    case LedColor::RED:
      return "red";
    case LedColor::YELLOW:
      return "yellow";
    case LedColor::PURPLE:
      return "purple";
    case LedColor::GREEN:
      return "green";
    case LedColor::NONE:
    default:
      return "none";
  }
}