# Sensor Servo - Cálculo de Delay

Este documento compara diferentes fórmulas para calcular el tiempo de delay necesario para que el servo llegue a su posición objetivo, considerando la comunicación serial entre ESP32-S3 y ATMega328p.

## Datos Reales Medidos

Se midieron los tiempos reales para diferentes deltas de ángulo:

| Δθ (grados) | Tiempo Real (ms) |
|-------------|------------------|
| 20°         | 300 ms          |
| 90°         | 430 ms          |
| 140°        | 600 ms          |

## Fórmulas Comparadas

### 1. Fórmula Original (Raíz Cuadrada)

```
t = 200 + 27 * sqrt(Δθ)
```

**Cálculos:**

- **Δθ = 20°**
  - √20 = 4.47
  - t = 200 + 27 × 4.47 = 200 + 120.7 = **320.7 ≈ 321 ms**

- **Δθ = 90°**
  - √90 = 9.49
  - t = 200 + 27 × 9.49 = 200 + 256.2 = **456.2 ≈ 456 ms**

- **Δθ = 140°**
  - √140 = 11.83
  - t = 200 + 27 × 11.83 = 200 + 319.4 = **519.4 ≈ 519 ms**

### 2. Fórmula Lineal Calibrada

```
t = 250 + 2.5 * Δθ
```

**Cálculos:**

- **Δθ = 20°**
  - t = 250 + 2.5 × 20 = **300 ms**

- **Δθ = 90°**
  - t = 250 + 2.5 × 90 = **475 ms**

- **Δθ = 140°**
  - t = 250 + 2.5 × 140 = **600 ms**

## Comparación de Resultados

| Δθ  | Tiempo Real | Raíz √ (Original) | Lineal (Calibrada) | Error Raíz | Error Lineal |
|-----|-------------|-------------------|-------------------|------------|--------------|
| 20° | 300 ms      | 321 ms            | 300 ms            | +21 ms     | 0 ms         |
| 90° | 430 ms      | 456 ms            | 475 ms            | +26 ms     | +45 ms       |
| 140°| 600 ms      | 519 ms            | 600 ms            | -81 ms     | 0 ms         |

## Análisis

- **Fórmula Lineal**: Coincide exactamente con los datos reales para 20° y 140°, y tiene un error de +45 ms para 90°.
- **Fórmula Raíz Cuadrada**: Tiene errores menores para ángulos pequeños/medianos, pero subestima significativamente (-81 ms) para ángulos grandes (140°).

## Implementación Actual

La función `calculateServoDelay()` en `sensor_servo.cpp` actualmente usa la fórmula original con raíz cuadrada. Para mejor precisión con los datos reales medidos, se recomienda considerar cambiar a la fórmula lineal calibrada.

### Ubicación del Código

```cpp
// lib/sensor_servo/sensor_servo.cpp
unsigned long SensorServo::calculateServoDelay(uint8_t currentAngle, uint8_t targetAngle)
{
  int delta_theta = abs(targetAngle - currentAngle);
  
  if (delta_theta == 0)
  {
    return 0;
  }
  
  // Fórmula actual (raíz cuadrada)
  unsigned long delay_ms = 200 + 27 * sqrt(delta_theta);
  return delay_ms;
  
  // Fórmula alternativa (lineal calibrada)
  // unsigned long delay_ms = 250 + 2.5 * delta_theta;
  // return delay_ms;
}
```

## Código de Test para Medición

El siguiente código se utilizó para medir los tiempos reales del servo. Se guarda aquí para futuras referencias y calibraciones.

**Ubicación:** `src/main/main.cpp`

```cpp
#include "car_actions/car_actions.h"
#include "elegoo_smartcar_lib.h"
#include "outputs/outputs.h"
#include "serial_comm/serial_comm.h"

#include <Arduino.h>
#include <ArduinoJson.h>

// Instancia del serialComm para comunicar con Atmega328p
SerialComm comm;

// Estructura de datos de salida (control atmega328p)
OutputData outputData;

// Configuración del test
const uint8_t START_ANGLE               = 20;
const uint8_t END_ANGLE                 = 160;
const uint8_t STEP_ANGLE                = 90;
const unsigned long DELAY_BETWEEN_MOVES = 600; // Tiempo entre movimientos (ms) - ajustar según necesites

unsigned long lastSendTime        = 0;
const unsigned long SEND_INTERVAL = 20; // Enviar cada 20ms como en el código real

void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, UART2_RX, UART2_TX);
  comm.initializeJsons();

  // Inicializar valores por defecto
  CarActions::freeStop(outputData);
  CarActions::setServoAngle(outputData, 90);
  CarActions::setLedColor(outputData, "YELLOW");

  // Esperar un poco para que se inicialice todo
  delay(1000);

  Serial.println("\n========================================");
  Serial.println("TEST SIMPLE DE SERVO - BARRIDO");
  Serial.println("========================================");
  Serial.print("Barrido de ");
  Serial.print(START_ANGLE);
  Serial.print("° a ");
  Serial.print(END_ANGLE);
  Serial.print("° en pasos de ");
  Serial.print(STEP_ANGLE);
  Serial.println("°");
  Serial.print("Tiempo entre movimientos: ");
  Serial.print(DELAY_BETWEEN_MOVES);
  Serial.println(" ms");
  Serial.println("\n>>> INICIANDO BARRIDO <<<\n");
}

void loop()
{
  // Enviar comandos periódicamente (como en el código real)
  unsigned long currentTime = millis();
  if (currentTime - lastSendTime >= SEND_INTERVAL)
  {
    // Actualizar sendJson desde outputData
    comm.sendJson["ledColor"]   = outputData.ledColor;
    comm.sendJson["servoAngle"] = outputData.servoAngle;

    // Actualizar objeto motors anidado
    JsonObject motors = comm.sendJson["motors"].to<JsonObject>();
    motors["action"]  = outputData.action;
    motors["speed"]   = outputData.speed;

    comm.sendJsonBySerial();
    lastSendTime = currentTime;
  }

  // Lógica del barrido: 20 → 160 → 20 → 160...
  static unsigned long lastMoveTime = 0;
  static uint8_t currentAngle       = START_ANGLE;
  static bool atStart               = true;

  if (currentTime - lastMoveTime >= DELAY_BETWEEN_MOVES)
  {
    // Mover servo al ángulo actual
    CarActions::setServoAngle(outputData, currentAngle);
    Serial.print("Servo -> ");
    Serial.print(currentAngle);
    Serial.print("° (START=");
    Serial.print(START_ANGLE);
    Serial.print(", END=");
    Serial.print(END_ANGLE);
    Serial.println(")");

    // Alternar entre 20 y 160
    if (atStart)
    {
      currentAngle = END_ANGLE; // Ir a 160
      atStart      = false;
    }
    else
    {
      currentAngle = START_ANGLE; // Volver a 20
      atStart      = true;
    }

    lastMoveTime = currentTime;
  }
}
```

### Uso del Test

1. Descomentar el código en `src/main/main.cpp`
2. Ajustar `DELAY_BETWEEN_MOVES` según sea necesario (empezar con 600 ms)
3. Observar físicamente el servo y verificar en el Serial Monitor los tiempos
4. Ajustar `DELAY_BETWEEN_MOVES` hasta encontrar el tiempo mínimo necesario
5. Registrar los tiempos reales para diferentes deltas de ángulo

## Notas

- Los tiempos incluyen la latencia de comunicación serial entre ESP32-S3 y ATMega328p.
- Los datos fueron medidos con un delay entre movimientos de 600 ms para asegurar que el servo llegara completamente a cada posición.
- La fórmula lineal es más simple y precisa para los casos extremos medidos.
- El código de test está comentado en `src/main/main.cpp` para no interferir con el código principal.
