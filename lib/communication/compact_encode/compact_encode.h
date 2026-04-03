#pragma once

#include <Arduino.h>

/**
 * Codificación compacta ESP32 → ATmega (UART JSON).
 * Equivalencias: lib/serial_comm/SERIAL_JSON_COMPACT_README.md
 */
namespace compact_encode
{
const char* motorActionToShort(const char* action);
const char* ledColorToShort(const String& color);
} // namespace compact_encode
