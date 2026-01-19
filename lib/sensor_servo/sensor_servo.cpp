#include "sensor_servo.h"

#include "../car_actions/car_actions.h"

#include <math.h>

SensorServo::SensorServo()
    : status(IDLE), previousStatus(IDLE), nextStatus(IDLE), scanningState(SCAN_START), currentAngle(FRONT_ANGLE),
      targetAngle(FRONT_ANGLE), startTurningTime(0), servoDelay(0), objectAngle(NO_OBJECT_FOUND),
      nextSearchAngle(MIN_ANGLE), searchIndex(0), minDistance(0), middleDistance(0), maxDistance(0)
{
}

void SensorServo::update(const InputData& inputData, OutputData& outputData)
{
  // Actualizar estado del m?dulo
  updateStatus();
  updateOutputs(inputData, outputData);
}

void SensorServo::updateStatus()
{
  if (status == TURNING)
  {
    if ((millis() - startTurningTime) >= servoDelay)
    {
      currentAngle   = targetAngle;
      previousStatus = status;
      status         = nextStatus;
      Serial.println((String) "SensorServo: TURNING->" + statusToString(nextStatus));
      return;
    }
  }
}

void SensorServo::updateOutputs(const InputData& inputData, OutputData& outputData)
{
  // Cuando el servo debe girar, usar CarActions para establecer el ángulo
  if (status == TURNING)
  {
    // Enviar comando al servo continuamente mientras gira
    CarActions::setServoAngle(outputData, targetAngle);
    return;
  }

  // Si ya terminó de girar, actualizar el ángulo actual en el output
  if (status != TURNING && currentAngle != outputData.servoAngle)
  {
    CarActions::setServoAngle(outputData, currentAngle);
  }

  // Manejo del estado SEARCHING
  if (status == SEARCHING)
  {
    // Usar la distancia del sensor desde InputData (telemetría constante)
    int distance = inputData.hcsr04DistanceCm;

    // OBJETO ENCONTRADO
    if (distance > 0 && distance <= SEARCHING_THRESHOOLD)
    {
      Serial.println((String) "SensorServo: OBJETO ENCONTRADO a " + distance + " cm en ángulo " + currentAngle);
      objectAngle = currentAngle;
      setAngle(FRONT_ANGLE, IDLE);
      return;
    }

    // Si no se encontró objeto, mover al siguiente ángulo
    if (objectAngle == NO_OBJECT_FOUND && status != TURNING)
    {
      nextSearchAngle = MIN_ANGLE + searchIndex * SEARCHING_STEP;
      if (nextSearchAngle > MAX_ANGLE)
      {
        nextSearchAngle = MIN_ANGLE;
        searchIndex     = 0;
      }
      setAngle(nextSearchAngle, SEARCHING);
      searchIndex++;
    }
  }

  // Manejo del estado SCANNING
  if (status == SCANNING)
  {
    int distance = inputData.hcsr04DistanceCm;

    switch (scanningState)
    {
      case SCAN_CENTER:
        // Medir distancia al centro (solo cuando no está girando)
        if (status != TURNING && currentAngle == FRONT_ANGLE)
        {
          if (distance > 0 && distance <= SEARCHING_THRESHOOLD)
          {
            middleDistance = distance;
            setAngle(MIN_ANGLE, SCANNING);
            scanningState = SCAN_LEFT;
            // Serial.println((String) "SensorServo SCANNING: SCAN_CENTER");
          }
        }
        break;

      case SCAN_LEFT:
        // Medir distancia a la izquierda (cuando termine de girar)
        // Nota: MIN_ANGLE (20°) está físicamente a la derecha, así que guardamos en maxDistance
        if (status != TURNING && currentAngle == MIN_ANGLE)
        {
          maxDistance = distance;
          setAngle(MAX_ANGLE, SCANNING);
          scanningState = SCAN_RIGHT;
          // Serial.println((String) "SensorServo SCANNING: SCAN_LEFT");
        }
        break;

      case SCAN_RIGHT:
        // Medir distancia a la derecha (cuando termine de girar)
        // Nota: MAX_ANGLE (160°) está físicamente a la izquierda, así que guardamos en minDistance
        if (status != TURNING && currentAngle == MAX_ANGLE)
        {
          minDistance   = distance;
          scanningState = SCAN_COMPLETE;
          setAngle(FRONT_ANGLE, SCANNING);
          Serial.println((String) "SensorServo: Resumen - IZQ: " + minDistance + " CENTRO: " + middleDistance +
                         " DER: " + maxDistance);
        }
        break;

      case SCAN_COMPLETE:
        // Escaneo completado, volver a centro cuando termine de girar
        if (status != TURNING && currentAngle == FRONT_ANGLE)
        {
          scanningState = SCAN_COMPLETE;
          // Serial.println((String) "SensorServo: ESCANEO COMPLETADO");
        }
        break;

      case SCAN_START:
        // Iniciar escaneo desde el centro
        scanningState = SCAN_CENTER;
        break;
    }
  }
}

void SensorServo::startScanning()
{
  if (status != IDLE)
  {
    return;
  }

  scanningState   = SCAN_CENTER;
  nextSearchAngle = FRONT_ANGLE;
  searchIndex     = 0;
  status          = SCANNING;
  Serial.println((String) "SensorServo: SCANNING");
}

void SensorServo::startSearching()
{
  if (status != IDLE)
  {
    return;
  }

  objectAngle     = NO_OBJECT_FOUND;
  nextSearchAngle = MIN_ANGLE;
  searchIndex     = 0;
  status          = SEARCHING;
  // Iniciar directamente el primer movimiento del servo
  setAngle(MIN_ANGLE, SEARCHING);
  Serial.println((String) "SensorServo: SEARCHING");
}

void SensorServo::stop()
{
  status        = IDLE;
  scanningState = SCAN_START;
  Serial.println((String) "SensorServo: IDLE");
}

void SensorServo::setAngle(uint8_t angle)
{
  setAngle(angle, IDLE);
}

void SensorServo::setAngle(uint8_t angle, SENSORSERVO_STATUS nextStatus)
{
  // Límites de ángulo
  if (angle < MIN_ANGLE)
  {
    angle = MIN_ANGLE;
  }
  if (angle > MAX_ANGLE)
  {
    angle = MAX_ANGLE;
  }

  // Si el ángulo es el mismo que el actual, no hacer nada
  if (angle == currentAngle)
  {
    return;
  }

  // Si ya está girando hacia ese ángulo, no hacer nada
  if (status == TURNING && angle == targetAngle)
  {
    return;
  }

  this->nextStatus = nextStatus;

  // Calcular tiempo usando regresión lineal
  servoDelay  = calculateServoDelay(currentAngle, angle);
  targetAngle = angle;

  // Si el delay es 0, no hay movimiento, actualizar directamente
  if (servoDelay == 0)
  {
    currentAngle   = angle;
    previousStatus = status;
    status         = nextStatus;
    return;
  }

  // Cambiar estado a TURNING solo si hay movimiento real
  previousStatus   = status;
  status           = TURNING;
  startTurningTime = millis();
  Serial.println((String) "SensorServo: TURNING -> " + angle + " delay: " + servoDelay);
}

unsigned long SensorServo::calculateServoDelay(uint8_t currentAngle, uint8_t targetAngle)
{
  // Fórmula final (simple, con fundamento físico):
  // t_ms = 100 + 27 * sqrt(Δθ)
  int delta_theta = abs(targetAngle - currentAngle);

  if (delta_theta == 0)
  {
    return 0;
  }

  unsigned long delay_ms = 200 + 27 * sqrt(delta_theta);
  return delay_ms;
}

String statusToString(SENSORSERVO_STATUS status)
{
  switch (status)
  {
    case IDLE:
      return "IDLE";
    case TURNING:
      return "TURNING";
    case SCANNING:
      return "SCANNING";
    case SEARCHING:
      return "SEARCHING";
    default:
      return "UNKNOWN";
  }
}
