#include "sensor_servo.h"

#include "../car_actions/car_actions.h"

#include <math.h>

SensorServo::SensorServo()
  : status(IDLE)
  , previousStatus(IDLE)
  , nextStatus(IDLE)
  , scanningState(SCAN_START)
  , searchingState(SEARCH_SWEEPING)
  , currentAngle(kFrontAngle)
  , targetAngle(kFrontAngle)
  , startTurningTime(0)
  , servoDelay(0)
  , objectAngle(kNoObjectFound)
  , lastFoundObjectAngle(kNoObjectFound)
  , nextSearchAngle(kMinAngle)
  , searchIndex(0)
  , needsReturnToCenter(false)
  , minDistance(0)
  , middleDistance(0)
  , maxDistance(0)
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
      // Solo logear cuando se sale de TURNING hacia IDLE (objeto encontrado, stop, etc.).
      // TURNING->SEARCHING ocurre en cada paso del barrido (~8+ veces/seg) y satura el Serial,
      // bloqueando el loop si el monitor no lee lo bastante rápido.
      if (nextStatus == IDLE)
      {
        // Serial.print("SensorServo: TURNING->");
        // Serial.println(statusToString(nextStatus));
      }
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
    switch (searchingState)
    {
      case SEARCH_SWEEPING:
        {
          // OBJETO ENCONTRADO: pasar a SEARCH_OBJECT_FOUND y llevar servo a centro
          if (distance > 0 && distance <= kSearchingThresholdCm)
          {
            objectAngle          = currentAngle;
            lastFoundObjectAngle = currentAngle;
            setAngle(kFrontAngle, IDLE);
            searchingState = SEARCH_OBJECT_FOUND;
            break;
          }

          // Si no se encontró objeto, mover al siguiente ángulo del barrido
          if (objectAngle == kNoObjectFound && status != TURNING)
          {
            if (needsReturnToCenter)
            {
              if (currentAngle == kFrontAngle)
              {
                nextSearchAngle     = kMinAngle;
                searchIndex         = 0;
                needsReturnToCenter = false;
                setAngle(nextSearchAngle, SEARCHING);
              }
              else
              {
                setAngle(kFrontAngle, SEARCHING);
              }
            }
            else
            {
              nextSearchAngle = kMinAngle + searchIndex * kSearchingStep;
              if (nextSearchAngle > kMaxAngle)
              {
                needsReturnToCenter = true;
                setAngle(kFrontAngle, SEARCHING);
              }
              else
              {
                setAngle(nextSearchAngle, SEARCHING);
                searchIndex++;
              }
            }
          }
        }
        break;

      case SEARCH_OBJECT_FOUND:
        // Objeto ya encontrado: setAngle(kFrontAngle, IDLE) pone status→TURNING y luego IDLE.
        // No hay más que hacer aquí; el FollowMode reacciona cuando servoStatus==IDLE.
        break;
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
        if (status != TURNING && currentAngle == kFrontAngle)
        {
          if (distance > 0 && distance <= kSearchingThresholdCm)
          {
            middleDistance = distance;
            setAngle(kMinAngle, SCANNING);
            scanningState = SCAN_LEFT;
            // Serial.println((String) "SensorServo SCANNING: SCAN_CENTER");
          }
        }
        break;

      case SCAN_LEFT:
        // Medir distancia a la izquierda (cuando termine de girar)
        // Nota: kMinAngle (20°) está físicamente a la derecha, así que guardamos en maxDistance
        if (status != TURNING && currentAngle == kMinAngle)
        {
          maxDistance = distance;
          setAngle(kMaxAngle, SCANNING);
          scanningState = SCAN_RIGHT;
          // Serial.println((String) "SensorServo SCANNING: SCAN_LEFT");
        }
        break;

      case SCAN_RIGHT:
        // Medir distancia a la derecha (cuando termine de girar)
        // Nota: kMaxAngle (160°) está físicamente a la izquierda, así que guardamos en minDistance
        if (status != TURNING && currentAngle == kMaxAngle)
        {
          minDistance   = distance;
          scanningState = SCAN_COMPLETE;
          setAngle(kFrontAngle, SCANNING);
          // Serial.println((String) "SensorServo: Resumen - IZQ: " + minDistance + " CENTRO: " + middleDistance +
          //  " DER: " + maxDistance);
        }
        break;

      case SCAN_COMPLETE:
        // Escaneo completado, volver a centro cuando termine de girar
        if (status != TURNING && currentAngle == kFrontAngle)
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
  nextSearchAngle = kFrontAngle;
  searchIndex     = 0;
  status          = SCANNING;
  // Serial.println((String) "SensorServo: SCANNING");
}

void SensorServo::startSearching()
{
  if (status != IDLE)
  {
    return;
  }

  objectAngle         = kNoObjectFound;
  nextSearchAngle     = kMinAngle;
  searchIndex         = 0;
  needsReturnToCenter = false;
  searchingState      = SEARCH_SWEEPING;
  status              = SEARCHING;

  // Si tenemos un ángulo donde encontramos objeto antes, empezar la búsqueda desde ahí
  if (lastFoundObjectAngle != kNoObjectFound)
  {
    // Ajustar al step de búsqueda más cercano y limitar al rango [kMinAngle, kMaxAngle]
    int idx = (lastFoundObjectAngle - kMinAngle + kSearchingStep / 2) / kSearchingStep;
    if (idx < 0)
      idx = 0;
    int maxIndex = (kMaxAngle - kMinAngle) / kSearchingStep;
    if (idx > maxIndex)
      idx = maxIndex;
    // searchIndex = idx+1 para que, al llegar a nextSearchAngle, el "siguiente" ya sea el paso posterior
    searchIndex     = idx + 1;
    nextSearchAngle = kMinAngle + idx * kSearchingStep;
    setAngle(nextSearchAngle, SEARCHING);
  }
  else
  {
    nextSearchAngle = kMinAngle;
    searchIndex     = 0;
    setAngle(kMinAngle, SEARCHING);
  }

  // Serial.println((String) "SensorServo: SEARCHING");
}

void SensorServo::stop()
{
  status         = IDLE;
  scanningState  = SCAN_START;
  searchingState = SEARCH_SWEEPING;
  // Serial.println((String) "SensorServo: IDLE");
}

void SensorServo::setAngle(uint8_t angle)
{
  setAngle(angle, IDLE);
}

void SensorServo::setAngle(uint8_t angle, SENSORSERVO_STATUS nextStatus)
{
  // Límites de ángulo
  if (angle < kMinAngle)
  {
    angle = kMinAngle;
  }
  if (angle > kMaxAngle)
  {
    angle = kMaxAngle;
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
  // Serial.println((String) "SensorServo: TURNING -> " + angle + " delay: " + servoDelay);
}

unsigned long SensorServo::calculateServoDelay(uint8_t currentAngle, uint8_t targetAngle)
{
  // Fórmula final (simple, con fundamento físico):
  // t_ms = 100 + 27 * sqrt(Δθ)
  int deltaTheta = abs(targetAngle - currentAngle);

  if (deltaTheta == 0)
  {
    return 0;
  }

  unsigned long delayMs = 150 + 27 * sqrt(deltaTheta);
  return delayMs;
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
