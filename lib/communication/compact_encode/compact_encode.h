#pragma once

#include <Arduino.h>

/**
 * Codificación compacta ESP32 → ATmega (UART JSON).
 * Equivalencias: lib/communication/SERIAL_JSON_COMPACT_README.md
 */
namespace CompactEncode
{
const char* motorActionToShort(const char* action);
const char* ledColorToShort(const String& color);
} // namespace CompactEncode
