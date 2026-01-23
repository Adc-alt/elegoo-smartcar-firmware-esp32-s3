#pragma once

#include "../car_actions/car_actions.h"
#include "../inputs/inputs.h"
#include "../mode_manager/mode_manager.h"
#include "../outputs/outputs.h"
#include "../sensor_servo/sensor_servo.h"

// Estados principales del modo de evasión de obstáculos
enum class ObstacleAvoidanceState
{
  MOVING_FORWARD, // Avanzando normalmente
  EVALUATING,     // Esperando escaneo y decidiendo dirección
  ESCAPING        // Ejecutando maniobra de escape (con subfases)
};

// Subfases de la secuencia de escape
enum class EscapePhase
{
  BACKUP,     // Retroceder
  TURN_LEFT,  // Girar a la izquierda
  TURN_RIGHT, // Girar a la derecha
  RESUME      // Volver a avanzar (transición a MOVING_FORWARD)
};

// Dirección de evasión
enum class AvoidDirection
{
  NONE  = 0,
  LEFT  = -1,
  RIGHT = 1
};

class ObstacleAvoidanceMode : public Mode
{
public:
  ObstacleAvoidanceMode();

  // Métodos para iniciar y detener el modo
  void startMode() override;
  void stopMode(OutputData& outputData) override;

  // Implementación de la interfaz Mode
  bool update(const InputData& inputData, OutputData& outputData) override;

private:
  // Referencias a módulos que este modo coordina
  SensorServo* sensorServo;

  // Estado de la máquina de estados
  ObstacleAvoidanceState currentState;

  // Tiempos de control para acciones temporales
  unsigned long turnStartTime;
  unsigned long backupStartTime;

  // Dirección elegida para evitar obstáculo
  AvoidDirection avoidDirection;

  // Estado de la secuencia de escape
  EscapePhase escapePhase;

  // Constantes
  static constexpr int OBSTACLE_THRESHOLD_CM          = 20;   // Umbral crítico para detenerse
  static constexpr int MIN_FREE_DISTANCE_CM           = 20;   // Distancia mínima para considerar dirección libre
  static constexpr uint8_t SPEED                      = 30;   // Velocidad del coche
  static constexpr unsigned long TURN_DURATION_MS     = 800;  // Tiempo para girar en evasión normal
  static constexpr unsigned long BACKUP_DURATION_MS   = 1000; // Tiempo para retroceder
  static constexpr unsigned long ESCAPE_TURN_LEFT_MS  = 800;  // Tiempo para girar izquierda en escape
  static constexpr unsigned long ESCAPE_TURN_RIGHT_MS = 300;  // Tiempo para girar derecha en escape
  static constexpr uint8_t SERVO_CENTER_ANGLE         = 90;   // Ángulo central del servo

  // Métodos privados
  void updateLogic(const InputData& inputData, OutputData& outputData);
  void decideDirection(OutputData& outputData);
  void handleEscapeSequence(unsigned long currentTime, const String& currentAction, OutputData& outputData);
  void resetServoToCenter();
  bool hasTimeElapsed(unsigned long startTime, unsigned long duration) const;
};
