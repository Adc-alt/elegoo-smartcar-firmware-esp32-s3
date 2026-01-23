#pragma once

#include "../car_actions/car_actions.h"
#include "../inputs/inputs.h"
#include "../mode_manager/mode_manager.h"
#include "../outputs/outputs.h"
#include "../sensor_servo/sensor_servo.h"

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

  // Tiempos de control para acciones temporales
  unsigned long turnStartTime;
  unsigned long backupStartTime;

  // Dirección elegida para evitar obstáculo
  int avoidDirection; // -1: izquierda, 1: derecha, 0: no definido

  // Flag para indicar si el modo está activo
  bool isActive;

  // Flag para rastrear secuencia de escape (retroceder -> girar izq -> girar der)
  bool isEscapeSequence;
  int escapePhase; // 0: retroceder, 1: girar izquierda, 2: girar derecha

  // Constantes
  static const int OBSTACLE_THRESHOLD_CM          = 20; // Umbral crítico para detenerse
  static const int MIN_FREE_DISTANCE_CM           = 20; // Distancia mínima para considerar dirección libre
  static const uint8_t SPEED                      = 30; // Velocidad del coche
  static const unsigned long TURN_DURATION_MS     = 800;
  static const unsigned long BACKUP_DURATION_MS   = 1000;
  static const unsigned long ESCAPE_TURN_LEFT_MS  = 800; // Tiempo para girar izquierda en escape
  static const unsigned long ESCAPE_TURN_RIGHT_MS = 300; // Tiempo para girar derecha en escape

  // Métodos privados
  void updateLogic(const InputData& inputData, OutputData& outputData);
  void decideDirection(OutputData& outputData);
};
