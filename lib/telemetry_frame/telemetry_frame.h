#pragma once
#include <Arduino.h>
#include <stdint.h>

/*
  TelemetryFrame (ESP32)
  ======================
  Representa el estado de los sensores recibido del ATmega.

  Espejo de la estructura en el ATmega, pero adaptada para ESP32.
  - Se actualiza cuando llega telemetría del ATmega
  - Es el punto de verdad único para el estado de sensores en el ESP32
*/

struct TelemetryFrame
{
  // ---- Metadatos ----
  uint32_t t_ms          = 0; // timestamp (millis desde el ATmega)
  uint16_t seq           = 0; // contador de secuencia
  uint32_t lastUpdate_ms = 0; // Última vez que se recibió (ESP32 time)

  // ---- Switch Button ----
  bool sw_pressed   = false;
  uint16_t sw_count = 0;

  // ---- HCSR04 ----
  uint16_t hcsr04_distanceCm   = 0;
  bool hcsr04_measurementValid = false;

  // ---- IrSensor ----
  bool ir_new     = false;
  uint32_t ir_raw = 0;
  String ir_data  = ""; // En ESP32 usamos String en lugar de const char*

  // ---- Battery ----
  float battery_voltage = 0.0f;

  // ---- Line Sensor ----
  uint16_t line_sensor_left   = 0;
  uint16_t line_sensor_middle = 0;
  uint16_t line_sensor_right  = 0;

  // ---- MPU ----
  float mpu_ax = 0.0f;
  float mpu_ay = 0.0f;
  float mpu_az = 0.0f;
  float mpu_gx = 0.0f;
  float mpu_gy = 0.0f;
  float mpu_gz = 0.0f;
};