#pragma once

#include "../car_actions/car_actions.h"
#include "../inputs/inputs.h"
#include "../mode_manager/mode_manager.h"
#include "../outputs/outputs.h"

/**
 * Modo de seguimiento de bola verde usando visión.
 * Recibe frames por web, analiza la imagen para detectar la bola verde,
 * y controla el coche para seguirla.
 */
class BallFollowMode : public Mode
{
public:
  BallFollowMode();
  void startMode() override;
  void stopMode(OutputData& outputData) override;
  bool update(const InputData& inputData, OutputData& outputData) override;

private:
  // TODO: Implementar detección de bola verde y lógica de seguimiento
  bool ballDetected;
  int ballCenterX;
  int ballCenterY;
};
