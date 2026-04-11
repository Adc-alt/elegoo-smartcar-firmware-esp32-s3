#pragma once

/**
 * Códigos raw del mando IR (decodificados en el ATmega).
 * Un solo sitio: ModeManager (selección de CarMode) e IrMode (conducción/servo).
 */
namespace IrRemote
{

// --- Selección de CarMode (ModeManager::trySelectModeFromIr) ---
constexpr unsigned long kSelectIdle       = 2907897600UL;
constexpr unsigned long kSelectIrMode     = 3910598400UL;
constexpr unsigned long kSelectObstacle   = 3860463360UL;
constexpr unsigned long kSelectFollow     = 4061003520UL;
constexpr unsigned long kSelectLine       = 4077715200UL;
constexpr unsigned long kSelectRc         = 3877175040UL;
constexpr unsigned long kSelectBallFollow = 2707357440UL;

// --- Conducción y servo (IrMode::processIrCommand) ---
constexpr unsigned long kDriveForward   = 3108437760UL;
constexpr unsigned long kDriveBackward  = 3927310080UL;
constexpr unsigned long kDriveTurnLeft  = 3141861120UL;
constexpr unsigned long kDriveTurnRight = 3158572800UL;
constexpr unsigned long kDriveStop      = 3208707840UL;
constexpr unsigned long kServoLeft      = 4144561920UL;
constexpr unsigned long kServoRight     = 2774204160UL;
constexpr unsigned long kServoCenter    = 3810328320UL;

} // namespace IrRemote
