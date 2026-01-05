// #include "telemetry_comm/telemetry_comm.h"

// TelemetryComm comm(Serial2);
// TelemetryData telemetry;

// void setup()
// {
//   Serial.begin(115200);
//   Serial2.begin(9600, SERIAL_8N1, UART2_RX, UART2_TX);
//   comm.begin();
// }

// void loop()
// {
//   // 1. LEER
//   if (comm.tryReceive(telemetry))
//   {
//     // Datos nuevos disponibles
//     Serial.print("irCommand: ");
//     Serial.println(telemetry.irCommand);
//   }

//   // 2. PROCESAR (aquí irá el sistema de modos)
//   // modeManager.run(telemetry);

//   // 3. ENVIAR
//   comm.send(telemetry);

//   // Timeout
//   if (comm.checkTimeout())
//   {
//     Serial.println("Timeout");
//   }
// }