#include "elegoo_smartcar_lib.h"
#include "telemetry_frame.h"
#include "telemetry_receiver.h"

#include <Arduino.h>

// Crear objetos
TelemetryFrame telemetryFrame;
TelemetryReceiver telemetryReceiver(Serial2); // Serial2 para recibir del ATmega

void setup()
{
  // Serial para debug/monitor (115200)
  Serial.begin(115200);
  delay(1000); // Dar tiempo para que se abra el monitor

  Serial.println("\n\n=== ESP32-S3 INICIANDO ===");
  Serial.println("ESP32 listo para recibir telemetría");

  // Serial2 para comunicación con ATmega (pines 40 TX, 3 RX)
  Serial2.begin(9600, SERIAL_8N1, UART2_RX, UART2_TX);
  delay(500);

  Serial.println("=== ESPERANDO DATOS DEL ATmega ===\n");
}

unsigned long lastHeartbeat = 0;

void loop()
{
  // Recibir telemetría del ATmega
  if (telemetryReceiver.tryReceive(telemetryFrame))
  {
    // Nuevo frame recibido
    Serial.print("Frame #");
    Serial.print(telemetryFrame.seq);
    Serial.print(" recibido - Battery: ");
    Serial.print(telemetryFrame.battery_voltage);
    Serial.println("V");

    // Ahora puedes usar los datos:
    // - telemetryFrame.battery_voltage está actualizado
    // - telemetryFrame.hcsr04_distanceCm tiene la distancia
    // - telemetryFrame.mpu_ax, mpu_ay, etc. tienen los valores del MPU
    // - etc.
  }

  // Verificar timeout (opcional)
  telemetryReceiver.checkTimeout(5000);

  delay(10);
}