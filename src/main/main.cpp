// #include "car_actions/car_actions.h"
// #include "elegoo_smartcar_lib.h"
// #include "outputs/outputs.h"
// #include "serial_comm/serial_comm.h"

// #include <Arduino.h>
// #include <ArduinoJson.h>

// // Instancia del serialComm para comunicar con Atmega328p
// SerialComm comm;

// // Estructura de datos de salida (control atmega328p)
// OutputData outputData;

// // Configuración del test
// const uint8_t START_ANGLE               = 20;
// const uint8_t END_ANGLE                 = 160;
// const uint8_t STEP_ANGLE                = 90;
// const unsigned long DELAY_BETWEEN_MOVES = 600; // Tiempo entre movimientos (ms) - ajustar según necesites

// unsigned long lastSendTime        = 0;
// const unsigned long SEND_INTERVAL = 20; // Enviar cada 20ms como en el código real

// void setup()
// {
//   Serial.begin(115200);
//   Serial2.begin(115200, SERIAL_8N1, UART2_RX, UART2_TX);
//   comm.initializeJsons();

//   // Inicializar valores por defecto
//   CarActions::freeStop(outputData);
//   CarActions::setServoAngle(outputData, 90);
//   CarActions::setLedColor(outputData, "YELLOW");

//   // Esperar un poco para que se inicialice todo
//   delay(1000);

//   Serial.println("\n========================================");
//   Serial.println("TEST SIMPLE DE SERVO - BARRIDO");
//   Serial.println("========================================");
//   Serial.print("Barrido de ");
//   Serial.print(START_ANGLE);
//   Serial.print("° a ");
//   Serial.print(END_ANGLE);
//   Serial.print("° en pasos de ");
//   Serial.print(STEP_ANGLE);
//   Serial.println("°");
//   Serial.print("Tiempo entre movimientos: ");
//   Serial.print(DELAY_BETWEEN_MOVES);
//   Serial.println(" ms");
//   Serial.println("\n>>> INICIANDO BARRIDO <<<\n");
// }

// void loop()
// {
//   // Enviar comandos periódicamente (como en el código real)
//   unsigned long currentTime = millis();
//   if (currentTime - lastSendTime >= SEND_INTERVAL)
//   {
//     // Actualizar sendJson desde outputData
//     comm.sendJson["ledColor"]   = outputData.ledColor;
//     comm.sendJson["servoAngle"] = outputData.servoAngle;

//     // Actualizar objeto motors anidado
//     JsonObject motors = comm.sendJson["motors"].to<JsonObject>();
//     motors["action"]  = outputData.action;
//     motors["speed"]   = outputData.speed;

//     comm.sendJsonBySerial();
//     lastSendTime = currentTime;
//   }

//   // Lógica del barrido: 20 → 160 → 20 → 160...
//   static unsigned long lastMoveTime = 0;
//   static uint8_t currentAngle       = START_ANGLE;
//   static bool atStart               = true;

//   if (currentTime - lastMoveTime >= DELAY_BETWEEN_MOVES)
//   {
//     // Mover servo al ángulo actual
//     CarActions::setServoAngle(outputData, currentAngle);
//     Serial.print("Servo -> ");
//     Serial.print(currentAngle);
//     Serial.print("° (START=");
//     Serial.print(START_ANGLE);
//     Serial.print(", END=");
//     Serial.print(END_ANGLE);
//     Serial.println(")");

//     // Alternar entre 20 y 160
//     if (atStart)
//     {
//       currentAngle = END_ANGLE; // Ir a 160
//       atStart      = false;
//     }
//     else
//     {
//       currentAngle = START_ANGLE; // Volver a 20
//       atStart      = true;
//     }

//     lastMoveTime = currentTime;
//   }
// }
