// lib/inputs/inputs.h
#pragma once

#include <Arduino.h>

// Datos de entrada (sensores y comandos recibidos)
struct InputData
{
  bool swPressed;
  int swCount;
  int hcsr04DistanceCm;
  int lineSensorLeft;
  int lineSensorMiddle;
  int lineSensorRight;
  float batVoltage;
  float mpuAccelX;
  float mpuAccelY;
  float mpuAccelZ;
  float mpuGyroX;
  float mpuGyroY;
  float mpuGyroZ;
  unsigned long irRaw;
};