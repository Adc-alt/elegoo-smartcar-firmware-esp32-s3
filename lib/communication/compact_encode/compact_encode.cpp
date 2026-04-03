#include "compact_encode.h"

#include <cstring>

namespace compact_encode
{

const char* motorActionToShort(const char* action)
{
  if (!action)
    return "fS";
  if (strcmp(action, "forward") == 0)
    return "fW";
  if (strcmp(action, "backward") == 0)
    return "bW";
  if (strcmp(action, "turnLeft") == 0)
    return "tL";
  if (strcmp(action, "turnRight") == 0)
    return "tR";
  if (strcmp(action, "freeStop") == 0)
    return "fS";
  if (strcmp(action, "forceStop") == 0)
    return "fT";
  return "fS";
}

const char* ledColorToShort(const String& color)
{
  if (color == "YELLOW")
    return "Y";
  if (color == "BLUE")
    return "B";
  if (color == "GREEN")
    return "G";
  if (color == "PURPLE")
    return "P";
  if (color == "WHITE")
    return "W";
  if (color == "SALMON")
    return "S";
  if (color == "CYAN")
    return "C";
  return "Y";
}

} // namespace compact_encode
