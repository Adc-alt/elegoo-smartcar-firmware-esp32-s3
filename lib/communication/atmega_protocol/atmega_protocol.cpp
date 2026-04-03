#include "atmega_protocol.h"

#include "compact_encode/compact_encode.h"

void fillInputFromAtmegaReceiveJson(const JsonDocument& receiveJson, InputData& inputData)
{
  inputData.swPressed        = receiveJson["swPressed"];
  inputData.swCount          = receiveJson["swCount"];
  inputData.hcsr04DistanceCm = receiveJson["hcsr04DistanceCm"];
  inputData.lineSensorLeft   = receiveJson["lineSensorLeft"];
  inputData.lineSensorMiddle = receiveJson["lineSensorMiddle"];
  inputData.lineSensorRight  = receiveJson["lineSensorRight"];
  inputData.batVoltage       = receiveJson["batVoltage"];
  inputData.mpuAccelX        = receiveJson["mpuAccelX"];
  inputData.mpuAccelY        = receiveJson["mpuAccelY"];
  inputData.mpuAccelZ        = receiveJson["mpuAccelZ"];
  inputData.mpuGyroX         = receiveJson["mpuGyroX"];
  inputData.mpuGyroY         = receiveJson["mpuGyroY"];
  inputData.mpuGyroZ         = receiveJson["mpuGyroZ"];
  inputData.irRaw            = receiveJson["irRaw"];
}

void fillSendJsonFromOutputs(JsonDocument& sendJson, const OutputData& outputData, int modeOrdinal)
{
  sendJson["sA"] = outputData.servoAngle;
  sendJson["lC"] = compact_encode::ledColorToShort(outputData.ledColor);
  sendJson["Md"] = modeOrdinal;

  JsonObject motors = sendJson["m"].to<JsonObject>();
  JsonObject left   = motors["L"].to<JsonObject>();
  JsonObject right  = motors["R"].to<JsonObject>();
  left["a"]         = compact_encode::motorActionToShort(outputData.leftAction.c_str());
  left["s"]         = outputData.leftSpeed;
  right["a"]        = compact_encode::motorActionToShort(outputData.rightAction.c_str());
  right["s"]        = outputData.rightSpeed;
}
