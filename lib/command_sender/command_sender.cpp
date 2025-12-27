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

  // Comando de vehículo (alto nivel) - enviar directamente sin traducir
  if (frame.vehicleAction != CarAction::NONE)
  {
    JsonObject car = actuators.createNestedObject("car");
    car["action"]  = carActionToString(frame.vehicleAction);
    car["speed"]   = frame.vehicleSpeed;
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
  // DEBUG: Mostrar JSON siempre
  Serial.print(F("[CommandSender] Enviando JSON: "));
  serializeJsonPretty(doc, Serial);
  Serial.println();
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
  // Función comentada - usar sendCarCommand en su lugar
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

void CommandSender::sendCarCommand(CarAction action, uint8_t speed)
{
  CommandFrame frame;
  frame.vehicleAction = action;
  frame.vehicleSpeed  = speed;
  send(frame);
}

// const char* CommandSender::motorActionToString(MotorAction action)
// {
//   switch (action)
//   {
//     case MotorAction::FORWARD:
//       return "forward";
//     case MotorAction::REVERSE:
//       return "reverse";
//     case MotorAction::FORCE_STOP:
//       return "force_stop";
//     case MotorAction::FREE_STOP:
//       return "free_stop";
//     case MotorAction::NONE:
//     default:
//       return "none";
//   }
// }

const char* CommandSender::carActionToString(CarAction action)
{
  switch (action)
  {
    case CarAction::FORWARD:
      return "forward";
    case CarAction::BACKWARD:
      return "backward";
    case CarAction::TURN_LEFT:
      return "turn_left";
    case CarAction::TURN_RIGHT:
      return "turn_right";
    case CarAction::FREE_STOP:
      return "free_stop";
    case CarAction::FORCE_STOP:
      return "force_stop";
    case CarAction::NONE:
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
