#include "telemetry_receiver.h"

TelemetryReceiver::TelemetryReceiver(Stream& in) : in(in)
{
  buffer.reserve(512); // Más grande que CommandReceiver porque telemetría es más grande
}

bool TelemetryReceiver::tryReceive(TelemetryFrame& telemetryFrame)
{
  while (in.available())
  {
    char c = in.read();

    if (!processingMessage)
    {
      processingMessage = true;
      buffer            = "";
    }

    if (c == '\n')
    {
      if (buffer.length() > 0)
      {
        StaticJsonDocument<512> doc; // Más grande para telemetría completa
        DeserializationError error = deserializeJson(doc, buffer);

// DEBUG: Mostrar JSON recibido
#ifdef DEBUG_TELEMETRY_RECEIVER
        Serial.print(F("[TelemetryReceiver] JSON recibido: "));
        Serial.println(buffer);
#endif

        if (error)
        {
#ifdef DEBUG_TELEMETRY_RECEIVER
          Serial.print(F("[TelemetryReceiver] Error parseando JSON: "));
          Serial.println(error.c_str());
#endif
        }
        else if (doc["type"] == "telemetry_frame")
        {
#ifdef DEBUG_TELEMETRY_RECEIVER
          Serial.println(F("[TelemetryReceiver] Telemetría válida recibida"));
#endif

          // Actualizar metadatos
          telemetryFrame.t_ms          = doc["t_ms"] | 0;
          telemetryFrame.seq           = doc["seq"] | 0;
          telemetryFrame.lastUpdate_ms = millis();

          // Parsear sensores
          if (doc.containsKey("sensors"))
          {
            JsonObject sensors = doc["sensors"];

            // Switch button
            if (sensors.containsKey("switch"))
            {
              JsonObject sw             = sensors["switch"];
              telemetryFrame.sw_pressed = sw["pressed"] | false;
              telemetryFrame.sw_count   = sw["count"] | 0;
            }

            // HCSR04
            if (sensors.containsKey("hcsr04"))
            {
              JsonObject hcsr04                      = sensors["hcsr04"];
              telemetryFrame.hcsr04_distanceCm       = hcsr04["distanceCm"] | 0;
              telemetryFrame.hcsr04_measurementValid = hcsr04["valid"] | false;
            }

            // IrSensor
            if (sensors.containsKey("irSensor"))
            {
              JsonObject irSensor   = sensors["irSensor"];
              telemetryFrame.ir_new = irSensor["new"] | false;
              telemetryFrame.ir_raw = irSensor["raw"] | 0;
              if (irSensor.containsKey("data") && !irSensor["data"].isNull())
              {
                telemetryFrame.ir_data = irSensor["data"].as<String>();
              }
            }

            // Battery
            if (sensors.containsKey("battery"))
            {
              JsonObject battery             = sensors["battery"];
              telemetryFrame.battery_voltage = battery["voltage"] | 0.0f;
#ifdef DEBUG_TELEMETRY_RECEIVER
              Serial.print(F("[TelemetryReceiver] Battery voltage: "));
              Serial.println(telemetryFrame.battery_voltage);
#endif
            }

            // Line Sensor
            if (sensors.containsKey("lineSensor"))
            {
              JsonObject lineSensor             = sensors["lineSensor"];
              telemetryFrame.line_sensor_left   = lineSensor["left"] | 0;
              telemetryFrame.line_sensor_middle = lineSensor["middle"] | 0;
              telemetryFrame.line_sensor_right  = lineSensor["right"] | 0;
            }

            // MPU
            if (sensors.containsKey("mpuSensor"))
            {
              JsonObject mpuSensor  = sensors["mpuSensor"];
              telemetryFrame.mpu_ax = mpuSensor["ax"] | 0.0f;
              telemetryFrame.mpu_ay = mpuSensor["ay"] | 0.0f;
              telemetryFrame.mpu_az = mpuSensor["az"] | 0.0f;
              telemetryFrame.mpu_gx = mpuSensor["gx"] | 0.0f;
              telemetryFrame.mpu_gy = mpuSensor["gy"] | 0.0f;
              telemetryFrame.mpu_gz = mpuSensor["gz"] | 0.0f;
            }
          }

          buffer            = "";
          processingMessage = false;
          return true;
        }
      }
      buffer            = "";
      processingMessage = false;
    }
    else if (c != '\r')
    {
      buffer += c;
    }
  }
  return false;
}

void TelemetryReceiver::checkTimeout(unsigned long timeoutMs)
{
  if (lastMessageTime > 0 && (millis() - lastMessageTime) > timeoutMs)
  {
    processingMessage = false;
    buffer            = "";
    lastMessageTime   = 0;
  }
}