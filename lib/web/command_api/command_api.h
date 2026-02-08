#ifndef COMMAND_API_H
#define COMMAND_API_H

#include "car_actions/car_actions.h"
#include "outputs/outputs.h"

/**
 * Capa de API de comandos: traduce acciones por nombre (ej. "forward", "stop")
 * a llamadas a CarActions sobre OutputData.
 * - Valida speed (0-255).
 * - Acciones soportadas: "forward", "backward", "stop", "left", "right".
 * - Acción desconocida se ignora (no modifica outputData).
 */
class CommandAPI
{
public:
  /** Ejecuta el comando (action + speed) sobre outputData. */
  static void execute(const char* action, int speed, OutputData& outputData);
};

#endif
