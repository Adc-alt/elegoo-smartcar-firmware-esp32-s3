#pragma once

#include <ArduinoJson.h>

#include "inputs/inputs.h"
#include "outputs/outputs.h"

// Contrato JSON UART con el ATmega (telemetría compacta / comandos compactos).
// Downlink: ver lib/communication/SERIAL_JSON_COMPACT_README.md. `Md` sale de `outputData.modeOrdinal`.

void fillInputFromAtmegaReceiveJson(const JsonDocument& receiveJson, InputData& inputData);
void fillSendJsonFromOutputs(JsonDocument& sendJson, const OutputData& outputData);
