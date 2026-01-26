#include "obstacle_avoidance.h"

#include <Arduino.h>

ObstacleAvoidanceMode::ObstacleAvoidanceMode()
  : sensorServo(nullptr)
  , currentState(ObstacleAvoidanceState::MOVING_FORWARD)
  , turnStartTime(0)
  , backupStartTime(0)
  , avoidDirection(AvoidDirection::NONE)
  , escapePhase(EscapePhase::BACKUP)
{
  // Crear instancia del sensor_servo (m?dulo con estados propios)
  // En una implementaci?n real, esto podr?a inyectarse desde fuera
  static SensorServo sensorServoInstance;
  sensorServo = &sensorServoInstance;
}

void ObstacleAvoidanceMode::startMode()
{
  // Resetear estado al iniciar el modo
  currentState    = ObstacleAvoidanceState::MOVING_FORWARD;
  avoidDirection  = AvoidDirection::NONE;
  escapePhase     = EscapePhase::BACKUP;
  turnStartTime   = 0;
  backupStartTime = 0;

  // Resetear servo a posici?n central
  if (sensorServo != nullptr)
  {
    sensorServo->stop();
    resetServoToCenter();
  }
  // Nota: La inicializaci?n del movimiento se hace en el primer update()
}

void ObstacleAvoidanceMode::stopMode(OutputData& outputData)
{
  // Limpiar estado al detener el modo
  currentState   = ObstacleAvoidanceState::MOVING_FORWARD;
  avoidDirection = AvoidDirection::NONE;
  escapePhase    = EscapePhase::BACKUP;

  // Detener servo
  if (sensorServo != nullptr)
  {
    sensorServo->stop();
    resetServoToCenter();
  }

  // Parar el coche al salir del modo
  CarActions::freeStop(outputData);
  CarActions::setServoAngle(outputData, SERVO_CENTER_ANGLE);
}

bool ObstacleAvoidanceMode::update(const InputData& inputData, OutputData& outputData)
{
  // Actualizar el m?dulo sensor_servo (gestiona sus propios estados)
  sensorServo->update(inputData, outputData);

  // L?gica del modo basada en estados de los actuadores
  updateLogic(inputData, outputData);

  // El modo est? siempre activo una vez iniciado
  return true;
}

void ObstacleAvoidanceMode::updateLogic(const InputData& inputData, OutputData& outputData)
{
  unsigned long currentTime      = millis();
  int distance                   = inputData.hcsr04DistanceCm;
  CarStatus carStatus            = CarActions::getStatus();
  SENSORSERVO_STATUS servoStatus = sensorServo->getStatus();
  const String& currentAction    = carStatus.currentAction;

  // M?quina de estados principal (3 estados de alto nivel)
  switch (currentState)
  {
    case ObstacleAvoidanceState::MOVING_FORWARD:
      // Mantener forward() y detectar obst?culos
      CarActions::forward(outputData, SPEED);

      if (distance > 0 && distance < OBSTACLE_THRESHOLD_CM)
      {
        Serial.println((String) "ObstacleAvoidance: Obstaculo detectado a " + distance + " cm - Iniciando escaneo");
        CarActions::forceStop(outputData);
        avoidDirection = AvoidDirection::NONE;
        currentState   = ObstacleAvoidanceState::EVALUATING;
      }
      // Mantener servo en centro durante avance normal
      else if (servoStatus == IDLE)
      {
        resetServoToCenter();
      }
      break;

    case ObstacleAvoidanceState::EVALUATING:

      if (!sensorServo->isScanComplete() && servoStatus == IDLE)
      {
        sensorServo->startScanning();
      }
      // Esperar a que termine el escaneo completamente (incluyendo retorno al centro del servo)
      // El sensorServo maneja su propio proceso: escanea y luego vuelve al centro autom?ticamente
      if (sensorServo->isScanComplete() && servoStatus != TURNING)
      {
        // El escaneo está completo y el servo ya terminó de moverse
        // Ahora podemos decidir la direcci?n sin interferir con el sensorServo
        decideDirection(outputData);
      }
      break;

    case ObstacleAvoidanceState::ESCAPING:
      // Manejar secuencia de escape con subfases
      handleEscapeSequence(currentTime, currentAction, outputData);
      break;
  }
}

void ObstacleAvoidanceMode::handleEscapeSequence(unsigned long currentTime, const String& currentAction,
                                                 OutputData& outputData)
{
  switch (escapePhase)
  {
    case EscapePhase::BACKUP:
      // Fase 1: Retroceder y luego girar a la izquierda
      // No hay que decidir dirección porque todas están bloqueadas
      if (currentAction == "backward" && hasTimeElapsed(backupStartTime, BACKUP_DURATION_MS))
      {
        Serial.println("ObstacleAvoidance: Retroceso completado - Girando izquierda");
        escapePhase    = EscapePhase::TURN_LEFT;
        turnStartTime  = millis();
        avoidDirection = AvoidDirection::LEFT;
        CarActions::turnLeft(outputData, SPEED);
        // En secuencia de escape, detener el escaneo si estaba activo
        sensorServo->stop();
        resetServoToCenter();
      }
      break;

    case EscapePhase::TURN_LEFT:
      // Girar a la izquierda y luego volver a avanzar
      if (currentAction == "turnLeft" && hasTimeElapsed(turnStartTime, TURN_DURATION_MS))
      {
        // Serial.println("ObstacleAvoidance: Giro izquierda completado - Reanudando avance");
        escapePhase = EscapePhase::RESUME;
      }
      break;

    case EscapePhase::TURN_RIGHT:
      // Girar a la derecha
      if (currentAction == "turnRight" && hasTimeElapsed(turnStartTime, TURN_DURATION_MS))
      {
        // Serial.println("ObstacleAvoidance: Giro derecha completado - Reanudando avance");
        escapePhase = EscapePhase::RESUME;
      }
      break;

    case EscapePhase::RESUME:
      // Fase final: Volver a avanzar y resetear a MOVING_FORWARD
      // Serial.println("ObstacleAvoidance: Secuencia completada - Avanzando");
      currentState    = ObstacleAvoidanceState::MOVING_FORWARD;
      escapePhase     = EscapePhase::BACKUP; // Resetear para pr?xima vez
      backupStartTime = 0;                   // Resetear para pr?xima vez
      CarActions::forward(outputData, SPEED);
      resetServoToCenter();
      break;
  }
}

void ObstacleAvoidanceMode::decideDirection(OutputData& outputData)
{
  // Obtener distancias desde el m?dulo sensor_servo
  int distanceLeft   = sensorServo->getMinDistance();
  int distanceCenter = sensorServo->getMiddleDistance();
  int distanceRight  = sensorServo->getMaxDistance();

  Serial.println((String) "ObstacleAvoidance: Evaluando direcciones - Izq: " + distanceLeft +
                 " cm, Centro: " + distanceCenter + " cm, Der: " + distanceRight + " cm");

  // Si las 3 direcciones tienen obst?culos muy cerca, iniciar secuencia de escape completa (zig-zag)
  if (distanceLeft < MIN_FREE_DISTANCE_CM && distanceCenter < MIN_FREE_DISTANCE_CM &&
      distanceRight < MIN_FREE_DISTANCE_CM)
  {
    Serial.println("ObstacleAvoidance: Todas las direcciones bloqueadas - Iniciando secuencia de escape completa");
    currentState    = ObstacleAvoidanceState::ESCAPING;
    escapePhase     = EscapePhase::BACKUP;
    backupStartTime = millis(); // Marcar inicio del escape completo (para distinguir de evasión simple)
    CarActions::backward(outputData, SPEED);
    sensorServo->stop();
    resetServoToCenter();
    return;
  }

  // Encontrar la direcci?n con m?s espacio libre (excluyendo el centro donde est? el obst?culo)
  int maxDistance              = 0;
  AvoidDirection bestDirection = AvoidDirection::NONE;

  // Comparar solo izquierda y derecha (no el centro)
  if (distanceLeft > maxDistance)
  {
    maxDistance   = distanceLeft;
    bestDirection = AvoidDirection::LEFT;
  }
  if (distanceRight > maxDistance)
  {
    maxDistance   = distanceRight;
    bestDirection = AvoidDirection::RIGHT;
  }

  // Girar hacia la mejor direcci?n (siempre izquierda o derecha, nunca centro)
  avoidDirection     = bestDirection;
  const char* dirStr = (bestDirection == AvoidDirection::LEFT) ? "IZQUIERDA" : "DERECHA";
  Serial.println((String) "ObstacleAvoidance: Mejor direccion es " + dirStr + " (" + maxDistance + " cm)");

  // Limpiar el estado del sensorServo despu?s de leer las distancias
  // Esto permite que el pr?ximo escaneo pueda iniciarse correctamente
  sensorServo->stop();

  // Transicionar a ESCAPING con fase TURN_LEFT o TURN_RIGHT según la dirección elegida
  // Esta es una evasión simple (un solo giro), no un escape completo
  currentState    = ObstacleAvoidanceState::ESCAPING;
  escapePhase     = (bestDirection == AvoidDirection::LEFT) ? EscapePhase::TURN_LEFT : EscapePhase::TURN_RIGHT;
  turnStartTime   = millis();
  backupStartTime = 0; // Asegurar que backupStartTime esté en 0 para evasión simple

  if (bestDirection == AvoidDirection::LEFT)
  {
    CarActions::turnLeft(outputData, SPEED);
  }
  else
  {
    CarActions::turnRight(outputData, SPEED);
  }
}

void ObstacleAvoidanceMode::resetServoToCenter()
{
  if (sensorServo != nullptr)
  {
    sensorServo->setAngle(SERVO_CENTER_ANGLE);
  }
}

bool ObstacleAvoidanceMode::hasTimeElapsed(unsigned long startTime, unsigned long duration) const
{
  unsigned long currentTime = millis();
  // Manejo seguro del overflow de millis() (cada ~49 d?as)
  if (currentTime < startTime)
  {
    // Overflow ocurri?, calcular el tiempo restante
    return (ULONG_MAX - startTime + currentTime) >= duration;
  }
  return (currentTime - startTime) >= duration;
}
