#pragma once

#include "../car_actions/car_actions.h"
#include "../inputs/inputs.h"
#include "../outputs/outputs.h"

#include <Arduino.h>

// Estados del módulo SensorServo (estados internos del módulo)
enum SENSORSERVO_STATUS
{
  IDLE,     // REPOSO: Servo parado y HC-SR04 en reposo
  TURNING,  // GIRANDO: Servo girando hasta el proximo punto para escanear
  SCANNING, // BARRIENDO: Servo girando cada SCANNING_STEP grados y escanenado
  SEARCHING // BUSCANDO: Servo girando cada SEARCHING_STEP grados
};

enum SCANNING_STATE
{
  SCAN_START,   // ESCANEO : Inicio
  SCAN_LEFT,    // ESCANEO : A la izquierda
  SCAN_CENTER,  // ESCANEO : Al centro
  SCAN_RIGHT,   // ESCANEO : A la derecha
  SCAN_COMPLETE // ESCANEO : Completo
};

class SensorServo
{
public:
  SensorServo();

  // Actualizar estado del módulo (se llama en cada loop)
  // Recibe InputData para obtener distancia del sensor y OutputData para comandos
  void update(const InputData& inputData, OutputData& outputData);

  // Getters
  SENSORSERVO_STATUS getStatus() const
  {
    return status;
  }
  uint8_t getCurrentAngle() const
  {
    return currentAngle;
  }
  int getSearchAngle() const
  {
    return objectAngle;
  } // Ángulo donde se encontró objeto (-1 si no)
  
  // Getters para distancias medidas durante el escaneo
  int getMinDistance() const
  {
    return minDistance;
  } // Distancia medida a la izquierda
  int getMiddleDistance() const
  {
    return middleDistance;
  } // Distancia medida al centro
  int getMaxDistance() const
  {
    return maxDistance;
  } // Distancia medida a la derecha
  
  // Verificar si el escaneo está completo
  bool isScanComplete() const
  {
    return scanningState == SCAN_COMPLETE;
  }

  // Métodos de control
  void startScanning();
  void startSearching();
  void stop();
  void setAngle(uint8_t angle);

private:
  // Estados internos del módulo
  SENSORSERVO_STATUS status;
  SENSORSERVO_STATUS previousStatus;
  SENSORSERVO_STATUS nextStatus;

  // Estado del escaneo
  SCANNING_STATE scanningState;

  // Configuración del servo
  uint8_t currentAngle;
  uint8_t targetAngle;
  unsigned long startTurningTime;
  unsigned long servoDelay;

  // Configuración de búsqueda
  int objectAngle; // Ángulo donde se encontró el objeto (NO_OBJECT_FOUND si no hay)
  uint8_t nextSearchAngle;
  int searchIndex;

  // Distancias medidas durante el escaneo
  int minDistance;    // Distancia mínima medida (izquierda)
  int middleDistance; // Distancia medida al centro
  int maxDistance;    // Distancia máxima medida (derecha)

  // Constantes
  static const uint8_t FRONT_ANGLE      = 90;
  static const uint8_t MIN_ANGLE        = 20;
  static const uint8_t MAX_ANGLE        = 160;
  static const uint8_t SEARCHING_STEP   = 10;
  static const int SEARCHING_THRESHOOLD = 30; // cm
  static const int NO_OBJECT_FOUND      = -1; // Valor que indica que no se encontró objeto

  // Métodos internos
  void updateStatus();
  void updateOutputs(const InputData& inputData, OutputData& outputData);
  void setAngle(uint8_t angle, SENSORSERVO_STATUS nextStatus);
  unsigned long calculateServoDelay(uint8_t currentAngle, uint8_t targetAngle);
};

String statusToString(SENSORSERVO_STATUS status);
