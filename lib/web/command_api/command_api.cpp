#include "command_api.h"

#include <cstring>

void CommandAPI::execute(const char* action, int speed, OutputData& outputData)
{
  if (action == nullptr)
    return;

  if (speed < 0)
    speed = 0;
  if (speed > 255)
    speed = 255;

  const uint8_t u8speed = static_cast<uint8_t>(speed);

  if (strcmp(action, "forward") == 0)
    CarActions::forward(outputData, u8speed);
  else if (strcmp(action, "backward") == 0)
    CarActions::backward(outputData, u8speed);
  else if (strcmp(action, "stop") == 0)
    CarActions::forceStop(outputData);
  else if (strcmp(action, "left") == 0)
    CarActions::turnLeft(outputData, u8speed);
  else if (strcmp(action, "right") == 0)
    CarActions::turnRight(outputData, u8speed);

  // Acción desconocida: no hacer nada
}
