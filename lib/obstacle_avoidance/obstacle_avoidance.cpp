#include "obstacle_avoidance.h"

#include <Arduino.h>

ObstacleAvoidanceMode::ObstacleAvoidanceMode()
  : sensorServo(nullptr)
  , turnStartTime(0)
  , backupStartTime(0)
  , avoidDirection(0)
  , isActive(false)
  , isEscapeSequence(false)
  , escapePhase(0)
{
  // Crear instancia del sensor_servo (m?dulo con estados propios)
  // En una implementaci?n real, esto podr?a inyectarse desde fuera
  static SensorServo sensorServoInstance;
  sensorServo = &sensorServoInstance;
}

void ObstacleAvoidanceMode::startMode()
{
  // Resetear estado al iniciar el modo
  isActive         = false; // Importante: resetear para que se active en updateLogic
  isEscapeSequence = false;
  escapePhase      = 0;
  avoidDirection   = 0;
  turnStartTime    = 0;
  backupStartTime  = 0;
  // Resetear servo a posición central
  if (sensorServo != nullptr)
  {
    sensorServo->stop();
    sensorServo->setAngle(90);
  }
  // Serial.println("ObstacleAvoidance: startMode() - Estado reseteado");
}

void ObstacleAvoidanceMode::stopMode(OutputData& outputData)
{
  // Limpiar estado al detener el modo
  isActive         = false;
  isEscapeSequence = false;
  escapePhase      = 0;
  avoidDirection   = 0;
  // Detener servo
  if (sensorServo != nullptr)
  {
    sensorServo->stop();
    sensorServo->setAngle(90);
  }
  // Parar el coche al salir del modo
  CarActions::freeStop(outputData);
  CarActions::setServoAngle(outputData, 90);
  // Serial.println("ObstacleAvoidance: stopMode() - Estado limpiado");
}

bool ObstacleAvoidanceMode::update(const InputData& inputData, OutputData& outputData)
{
  // Actualizar el m?dulo sensor_servo (gestiona sus propios estados)
  sensorServo->update(inputData, outputData);

  // L?gica del modo basada en estados de los actuadores
  updateLogic(inputData, outputData);

  return isActive;
}

void ObstacleAvoidanceMode::updateLogic(const InputData& inputData, OutputData& outputData)
{
  unsigned long currentTime      = millis();
  int distance                   = inputData.hcsr04DistanceCm;
  CarStatus carStatus            = CarActions::getStatus();
  SENSORSERVO_STATUS servoStatus = sensorServo->getStatus();

  // Activar modo si no est? activo
  if (!isActive)
  {
    isActive = true;
    // Iniciar movimiento
    CarActions::forward(outputData, SPEED);
    sensorServo->setAngle(90); // Centro
    return;
  }

  // L?gica basada en estados de los actuadores
  String currentAction = carStatus.currentAction;

  // Si el coche est? avanzando y detecta obst?culo
  if (currentAction == "forward" && distance > 0 && distance < OBSTACLE_THRESHOLD_CM)
  {
    Serial.println((String) "ObstacleAvoidance: Obst?culo detectado a " + distance + " cm - Iniciando escaneo");
    CarActions::forceStop(outputData);
    sensorServo->startScanning();
    avoidDirection = 0;
  }
  // Si el sensor_servo est? escaneando, esperar a que termine
  else if (servoStatus == SCANNING)
  {
    // El sensor_servo gestiona su propio estado
    // Cuando termine, decidiremos la direcci?n
    if (sensorServo->isScanComplete())
    {
      decideDirection(outputData);
    }
  }
  // Si estamos en secuencia de escape
  if (isEscapeSequence)
  {
    if (currentAction == "backward" && (currentTime - backupStartTime) >= BACKUP_DURATION_MS)
    {
      // Fase 1: Girar a la izquierda
      Serial.println("ObstacleAvoidance: Retroceso completado - Girando izquierda");
      escapePhase   = 1;
      turnStartTime = millis();
      CarActions::turnLeft(outputData, SPEED);
      sensorServo->stop();
      sensorServo->setAngle(90); // Centro
    }
    else if (escapePhase == 1 && currentAction == "turnLeft" && (currentTime - turnStartTime) >= ESCAPE_TURN_LEFT_MS)
    {
      // Fase 2: Girar un poco a la derecha
      Serial.println("ObstacleAvoidance: Giro izquierda completado - Girando derecha");
      escapePhase   = 2;
      turnStartTime = millis();
      CarActions::turnRight(outputData, SPEED);
      sensorServo->setAngle(90); // Centro
    }
    else if (escapePhase == 2 && currentAction == "turnRight" && (currentTime - turnStartTime) >= ESCAPE_TURN_RIGHT_MS)
    {
      // Fase 3: Finalizar escape y avanzar
      Serial.println("ObstacleAvoidance: Secuencia de escape completada - Avanzando");
      isEscapeSequence = false;
      escapePhase      = 0;
      CarActions::forward(outputData, SPEED);
      sensorServo->setAngle(90); // Centro
    }
    return;
  }

  // Si el coche est? girando, verificar tiempo
  else if (currentAction == "turnLeft" || currentAction == "turnRight")
  {
    if ((currentTime - turnStartTime) >= TURN_DURATION_MS)
    {
      Serial.println("ObstacleAvoidance: Giro completado - Avanzando");
      CarActions::forward(outputData, SPEED);
      sensorServo->setAngle(90); // Centro
    }
  }
  // Si el coche est? retrocediendo, verificar tiempo
  else if (currentAction == "backward")
  {
    if ((currentTime - backupStartTime) >= BACKUP_DURATION_MS)
    {
      Serial.println("ObstacleAvoidance: Retroceso completado - Avanzando");
      CarActions::forward(outputData, SPEED);
      sensorServo->setAngle(90); // Centro
    }
  }
  // Si el coche est? avanzando y el servo est? en IDLE, mantener servo en centro
  else if (currentAction == "forward" && servoStatus == IDLE)
  {
    sensorServo->setAngle(90); // Centro
  }
}

void ObstacleAvoidanceMode::decideDirection(OutputData& outputData)
{
  // Obtener distancias desde el módulo sensor_servo
  int distanceLeft   = sensorServo->getMinDistance();
  int distanceCenter = sensorServo->getMiddleDistance();
  int distanceRight  = sensorServo->getMaxDistance();

  Serial.println((String) "ObstacleAvoidance: Evaluando direcciones - Izq: " + distanceLeft +
                 " cm, Centro: " + distanceCenter + " cm, Der: " + distanceRight + " cm");

  // Si las 3 direcciones tienen obstáculos muy cerca, iniciar secuencia de escape
  if (distanceLeft < MIN_FREE_DISTANCE_CM && distanceCenter < MIN_FREE_DISTANCE_CM &&
      distanceRight < MIN_FREE_DISTANCE_CM)
  {
    Serial.println("ObstacleAvoidance: Todas las direcciones bloqueadas - Iniciando secuencia de escape");
    isEscapeSequence          = true;
    escapePhase               = 0; // Empezar retrocediendo
    unsigned long currentTime = millis();
    backupStartTime           = currentTime;
    CarActions::backward(outputData, SPEED);
    sensorServo->stop();
    sensorServo->setAngle(90); // Centro
    return;
  }

  // Encontrar la direcci?n con m?s espacio libre (excluyendo el centro donde está el obstáculo)
  int maxDistance   = 0;
  int bestDirection = -1; // Solo izquierda o derecha, nunca centro

  // Comparar solo izquierda y derecha (no el centro)
  if (distanceLeft > maxDistance)
  {
    maxDistance   = distanceLeft;
    bestDirection = -1;
  }
  if (distanceRight > maxDistance)
  {
    maxDistance   = distanceRight;
    bestDirection = 1;
  }

  // Girar hacia la mejor direcci?n (siempre izquierda o derecha, nunca centro)
  avoidDirection = bestDirection;
  String dirStr  = (bestDirection == -1) ? "IZQUIERDA" : "DERECHA";
  Serial.println((String) "ObstacleAvoidance: Mejor direccion es " + dirStr + " (" + maxDistance + " cm) ");

  unsigned long currentTime = millis();
  turnStartTime             = currentTime;

  if (bestDirection == -1)
  {
    CarActions::turnLeft(outputData, SPEED);
  }
  else
  {
    CarActions::turnRight(outputData, SPEED);
  }
  sensorServo->stop();
  sensorServo->setAngle(90); // Centro
}
