#pragma once

#include <ArduinoJson.h>

#include "inputs/inputs.h"
#include "outputs/outputs.h"

// Contrato JSON UART con el ATmega (telemetría compacta / comandos compactos).
// Downlink: ver serial_comm/SERIAL_JSON_COMPACT_README.md. modeOrdinal = (int)CarMode.

void fillInputFromAtmegaReceiveJson(const JsonDocument& receiveJson, InputData& inputData);
void fillSendJsonFromOutputs(JsonDocument& sendJson, const OutputData& outputData, int modeOrdinal);
