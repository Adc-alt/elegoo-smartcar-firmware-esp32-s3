#include "follow_mode.h"

#include <Arduino.h>
#include <math.h>

FollowMode::FollowMode()
  : sensorServo(nullptr)
  , currentState(FollowModeState::SEARCHING)
  , turnStartTime(0)
  , turnDuration(0)
  , servoResetAfterTurn(false)
  , foundObjectAngle(SensorServo::NO_OBJECT_FOUND)
  , searchingStartedLogged(false)
  , lastLogTime(0)
{
  // Crear instancia del sensor_servo (módulo con estados propios)
  // En una implementación real, esto podría inyectarse desde fuera
  static SensorServo sensorServoInstance;
  sensorServo = &sensorServoInstance;
}

void FollowMode::startMode()
{
  // Resetear estado al iniciar el modo
  currentState           = FollowModeState::SEARCHING;
  turnStartTime          = 0;
  turnDuration           = 0;
  servoResetAfterTurn    = false;
  foundObjectAngle       = SensorServo::NO_OBJECT_FOUND;
  searchingStartedLogged = false;
  lastLogTime            = 0;

  // Detener cualquier acción previa del servo y resetear a centro
  if (sensorServo != nullptr)
  {
    sensorServo->stop();
    resetServoToCenter();
  }

  Serial.println("FollowMode: Modo iniciado - Buscando objeto...");
}

void FollowMode::stopMode(OutputData& outputData)
{
  // Limpiar estado al detener el modo
  currentState           = FollowModeState::SEARCHING;
  turnStartTime          = 0;
  turnDuration           = 0;
  servoResetAfterTurn    = false;
  foundObjectAngle       = SensorServo::NO_OBJECT_FOUND;
  searchingStartedLogged = false;
  lastLogTime            = 0;

  // Detener servo
  if (sensorServo != nullptr)
  {
    sensorServo->stop();
    resetServoToCenter();
  }

  // Parar el coche al salir del modo
  CarActions::freeStop(outputData);
  CarActions::setServoAngle(outputData, SensorServo::FRONT_ANGLE);
}

bool FollowMode::update(const InputData& inputData, OutputData& outputData)
{
  // Actualizar el módulo sensor_servo (gestiona sus propios estados)
  sensorServo->update(inputData, outputData);

  // Lógica del modo basada en estados
  updateLogic(inputData, outputData);

  // El modo está siempre activo una vez iniciado
  return true;
}

void FollowMode::updateLogic(const InputData& inputData, OutputData& outputData)
{
  unsigned long currentTime      = millis();
  int distance                   = inputData.hcsr04DistanceCm;
  SENSORSERVO_STATUS servoStatus = sensorServo->getStatus();
  int objectAngle                = sensorServo->getSearchAngle();

  // Máquina de estados principal
  switch (currentState)
  {
    case FollowModeState::SEARCHING:
      {
        // Iniciar búsqueda si el servo está en IDLE
        if (servoStatus == IDLE)
        {
          sensorServo->startSearching();
          // Solo imprimir la primera vez que se inicia la búsqueda para evitar saturar Serial
          if (!searchingStartedLogged)
          {
            Serial.println("FollowMode: SEARCHING - Iniciando barrido de búsqueda...");
            searchingStartedLogged = true;
          }
        }
        // Resetear el flag cuando el servo sale de IDLE (empieza a buscar)
        else if (servoStatus != IDLE)
        {
          searchingStartedLogged = false;
        }

        // Verificar si se encontró un objeto
        // El sensor_servo automáticamente vuelve a FRONT_ANGLE cuando encuentra un objeto
        if (objectAngle != SensorServo::NO_OBJECT_FOUND && servoStatus == IDLE)
        {
          // Guardar el ángulo del objeto antes de iniciar el giro
          foundObjectAngle = objectAngle;
          Serial.println((String) "FollowMode: SEARCHING - Objeto encontrado en ángulo " + foundObjectAngle + "°");

          // Si el objeto está al frente (90°), ir directamente hacia delante sin girar
          if (foundObjectAngle == SensorServo::FRONT_ANGLE)
          {
            Serial.println("FollowMode: Objeto al frente (90°), avanzando directamente");
            currentState = FollowModeState::MOVING_FORWARD;
          }
          else
          {
            // Objeto no está al frente, necesita girar
            Serial.println("FollowMode: Transicionando a TURNING_TO_OBJECT");

            // Calcular duración del giro basándose en el ángulo
            // Ángulo relativo al centro: |objectAngle - 90|
            // El rango máximo es 70° (de 20° a 90° o de 90° a 160°)
            int angleFromCenter = abs(foundObjectAngle - SensorServo::FRONT_ANGLE);
            // Calcular tiempo de giro proporcional al ángulo
            // Reducir la base para que gire menos y se alinee mejor
            // Usar 600ms como base para el ángulo máximo (70°), reducido desde 800ms
            // Fórmula: tiempo = base * (ángulo / ángulo_máximo) * factor_ajuste
            // Factor de ajuste: 0.85 para reducir un 15% el tiempo de giro
            turnDuration = (unsigned long)(400 * (angleFromCenter / 70.0) * 0.85);
            if (turnDuration < 200)
            {
              turnDuration = 200; // Mínimo 200ms para giros pequeños
            }
            if (turnDuration > 1200)
            {
              turnDuration = 1200; // Máximo 1.2 segundos (reducido desde 1.5s)
            }

            Serial.println((String) "FollowMode: Ángulo desde centro: " + angleFromCenter +
                           "°, Duración giro: " + turnDuration + "ms");

            // Iniciar giro del coche hacia el objeto
            turnCarToAngle(foundObjectAngle, outputData);
            turnStartTime       = currentTime;
            servoResetAfterTurn = false;
            currentState        = FollowModeState::TURNING_TO_OBJECT;
          }
        }
      }
      break;

    case FollowModeState::TURNING_TO_OBJECT:
      {
        unsigned long elapsed = currentTime - turnStartTime;

        // Fase 1: Esperar a que termine el giro del coche
        if (elapsed < turnDuration)
        {
          // El coche sigue girando, no hacer nada más
          // (el comando de giro ya se envió en turnCarToAngle)
        }
        // Fase 2: Después de girar, resetear servo a centro y verificar objeto
        else if (!servoResetAfterTurn)
        {
          Serial.println("FollowMode: TURNING_TO_OBJECT - Giro completado, reseteando servo a centro");
          CarActions::forceStop(outputData);
          resetServoToCenter();
          servoResetAfterTurn = true;
          // Dar un pequeño tiempo para que el servo se mueva al centro
          turnStartTime = currentTime; // Reiniciar timer para la verificación
        }
        // Fase 3: Verificar que el objeto sigue ahí después de girar
        else
        {
          // Esperar un momento para que el servo se estabilice y luego verificar distancia
          if (elapsed >= 500) // Esperar 500ms después de resetear el servo
          {
            if (distance > 0 && distance <= SensorServo::SEARCHING_THRESHOOLD)
            {
              Serial.println((String) "FollowMode: TURNING_TO_OBJECT - Objeto confirmado a " + distance +
                             " cm, iniciando seguimiento");
              currentState = FollowModeState::MOVING_FORWARD;
            }
            else
            {
              Serial.println(
                "FollowMode: TURNING_TO_OBJECT - Objeto no encontrado después del giro, reiniciando búsqueda");
              // Reiniciar búsqueda
              if (sensorServo != nullptr)
              {
                sensorServo->stop();
                resetServoToCenter();
              }
              currentState = FollowModeState::SEARCHING;
            }
          }
        }
      }
      break;

    case FollowModeState::MOVING_FORWARD:
      {
        // Mantener servo en centro durante el seguimiento
        if (servoStatus == IDLE)
        {
          resetServoToCenter();
        }

        // Verificar distancia al objeto
        if (distance > 0)
        {
          // Objeto perdido: distancia mayor que el umbral
          if (distance > OBJECT_LOST_DISTANCE_CM)
          {
            Serial.println((String) "FollowMode: MOVING_FORWARD - Objeto perdido (distancia: " + distance +
                           " cm), reiniciando búsqueda");
            CarActions::forceStop(outputData);
            foundObjectAngle       = SensorServo::NO_OBJECT_FOUND;
            currentState           = FollowModeState::SEARCHING;
            searchingStartedLogged = false; // Resetear flag para permitir log en próxima búsqueda
            if (sensorServo != nullptr)
            {
              sensorServo->stop();
              resetServoToCenter();
            }
          }
          // Objeto muy cerca: detenerse
          else if (distance <= OBJECT_TOO_CLOSE_CM)
          {
            CarActions::forceStop(outputData);
            Serial.println((String) "FollowMode: MOVING_FORWARD - Objeto muy cerca (" + distance + " cm), deteniendo");
          }
          // Objeto en rango: avanzar hacia él
          else if (distance <= SensorServo::SEARCHING_THRESHOOLD)
          {
            CarActions::forward(outputData, SPEED);
            // Log periódico para no saturar
            static unsigned long lastLogTime = 0;
            if (currentTime - lastLogTime >= 1000)
            {
              Serial.println((String) "FollowMode: MOVING_FORWARD - Siguiendo objeto a " + distance + " cm");
              lastLogTime = currentTime;
            }
          }
        }
        else
        {
          // No hay lectura válida del sensor, detener y buscar
          Serial.println("FollowMode: MOVING_FORWARD - Sin lectura del sensor, reiniciando búsqueda");
          CarActions::forceStop(outputData);
          foundObjectAngle       = SensorServo::NO_OBJECT_FOUND;
          currentState           = FollowModeState::SEARCHING;
          searchingStartedLogged = false; // Resetear flag para permitir log en próxima búsqueda
          if (sensorServo != nullptr)
          {
            sensorServo->stop();
            resetServoToCenter();
          }
        }
      }
      break;
  }
}

void FollowMode::resetServoToCenter()
{
  if (sensorServo != nullptr)
  {
    sensorServo->setAngle(SensorServo::FRONT_ANGLE);
  }
}

void FollowMode::turnCarToAngle(int objectAngle, OutputData& outputData)
{
  // El ángulo del objeto está entre 20° (MIN_ANGLE) y 160° (MAX_ANGLE)
  // 90° es el centro (FRONT_ANGLE)
  // IMPORTANTE: MIN_ANGLE (20°) está físicamente a la DERECHA, MAX_ANGLE (160°) está físicamente a la IZQUIERDA

  // Convertir ángulo del servo a dirección física
  // Ángulos pequeños (20-90°) = físicamente a la derecha → girar derecha
  // Ángulos grandes (90-160°) = físicamente a la izquierda → girar izquierda

  if (objectAngle < SensorServo::FRONT_ANGLE)
  {
    // Objeto físicamente a la DERECHA (MIN_ANGLE está a la derecha), girar a la derecha
    Serial.println((String) "FollowMode: Girando DERECHA hacia objeto (ángulo servo: " + objectAngle +
                   "° = físicamente DERECHA)");
    CarActions::turnRight(outputData, SPEED);
  }
  else if (objectAngle > SensorServo::FRONT_ANGLE)
  {
    // Objeto físicamente a la IZQUIERDA (MAX_ANGLE está a la izquierda), girar a la izquierda
    Serial.println((String) "FollowMode: Girando IZQUIERDA hacia objeto (ángulo servo: " + objectAngle +
                   "° = físicamente IZQUIERDA)");
    CarActions::turnLeft(outputData, SPEED);
  }
  else
  {
    // Si objectAngle == 90, el objeto está al frente, no girar
    Serial.println("FollowMode: Objeto al frente (90°), no girar");
    // No girar, pasar directamente a MOVING_FORWARD
    currentState = FollowModeState::MOVING_FORWARD;
  }
}
