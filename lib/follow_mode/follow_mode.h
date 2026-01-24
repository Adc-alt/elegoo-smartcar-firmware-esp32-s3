#pragma once

#include "../car_actions/car_actions.h"
#include "../inputs/inputs.h"
#include "../mode_manager/mode_manager.h"
#include "../outputs/outputs.h"
#include "../sensor_servo/sensor_servo.h"

// Estados principales del modo de seguimiento
enum class FollowModeState
{
  SEARCHING,
  TURNING_TO_OBJECT,
  MOVING_FORWARD
};

class FollowMode : public Mode
{
public:
  FollowMode();

  // Métodos para iniciar y detener en modo
  void startMode() override;
  void stopMode(OutputData& outputData) override;

  //
  bool update(const InputData& inputData, OutputData& outputData) override;

private:
  SensorServo* sensorServo;

  FollowModeState currentState;

  // Tiempo de control para el giro
  unsigned long turnStartTime;
  unsigned long turnDuration;
  bool servoResetAfterTurn; // Flag para saber si ya se reseteó el servo después del giro
  int foundObjectAngle;     // Ángulo donde se encontró el objeto (guardado para el giro)

  // Constantes
  static constexpr uint8_t SPEED                  = 30;  // Velocidad del coche
  static constexpr unsigned long TURN_DURATION_MS = 100; // Tiempo para girar hacia el objeto
  static constexpr int OBJECT_LOST_DISTANCE_CM =
    36; // Distancia a la que se considera que se perdió el objeto (mayor que umbral de detección)
  static constexpr int OBJECT_TOO_CLOSE_CM = 5; // Distancia mínima para seguir avanzando

  // Métodos privados
  void updateLogic(const InputData& inputData, OutputData& outputData);
  void resetServoToCenter();
  void turnCarToAngle(int objectAngle, OutputData& outputData);
};