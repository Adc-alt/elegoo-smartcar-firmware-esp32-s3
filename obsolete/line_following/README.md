# Line Following Mode

Modo de seguimiento de línea para el Elegoo Smart Car.

## ⚠️ Problema Crítico: Saturación del Buffer Serial

### Problema Identificado

El bloqueo del sistema puede ocurrir debido a la **saturación del buffer Serial**. Este problema se manifestó en `follow_mode.cpp` y puede afectar a cualquier modo que imprima mensajes Serial repetitivamente.

#### Síntomas
- El sistema se bloquea después de un tiempo de funcionamiento
- Los mensajes Serial dejan de aparecer
- El loop principal se detiene
- El problema solo ocurre cuando ha pasado un rato (el buffer se llena gradualmente)

#### Causa Raíz

En `sensor_servo.cpp` (líneas 44-46) ya existe un comentario al respecto:
```cpp
// TURNING->SEARCHING ocurre en cada paso del barrido (~8+ veces/seg) y satura el Serial,
// bloqueando el loop si el monitor no lee lo bastante rápido.
```

El problema ocurre cuando:
1. Se imprimen mensajes `Serial.println()` dentro de bucles que se ejecutan frecuentemente
2. El buffer Serial (típicamente 256 bytes en ESP32) se llena más rápido de lo que se puede vaciar
3. Cuando el buffer está lleno, `Serial.println()` se bloquea esperando espacio
4. Esto detiene el loop principal y congela el sistema

### Solución Implementada (FollowMode)

En `follow_mode.cpp` se implementó la siguiente solución:

#### 1. Flag de Control de Logs
```cpp
// En follow_mode.h
bool searchingStartedLogged; // Flag para evitar imprimir "Iniciando barrido" repetidamente
unsigned long lastLogTime;   // Último tiempo de log para mensajes periódicos
```

#### 2. Control Condicional de Impresiones
```cpp
// En follow_mode.cpp línea 88-99
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
```

#### 3. Reset del Flag en Transiciones de Estado
El flag se resetea cuando:
- El servo sale de `IDLE` (empieza a buscar)
- Se reinicia la búsqueda desde otros estados (MOVING_FORWARD, TURNING_TO_OBJECT)

### Aplicación en LineFollowingMode

**⚠️ IMPORTANTE:** Al implementar completamente `LineFollowingMode`, aplicar las siguientes buenas prácticas:

#### 1. Evitar Serial.println() en Bucles Frecuentes

❌ **MAL:**
```cpp
void LineFollowingMode::updateLogic(...)
{
  switch (currentState)
  {
    case LineFollowingModeState::MOVING_FORWARD:
      Serial.println("Siguiendo línea..."); // ❌ Se ejecuta cada loop (~100+ veces/seg)
      break;
  }
}
```

✅ **BIEN:**
```cpp
// En line_following.h
private:
  bool stateChangeLogged;
  unsigned long lastLogTime;
  LineFollowingModeState previousState;

// En line_following.cpp
void LineFollowingMode::updateLogic(...)
{
  unsigned long currentTime = millis();
  
  // Solo imprimir cuando cambia el estado
  if (currentState != previousState)
  {
    Serial.println("LineFollowingMode: Cambiando estado...");
    stateChangeLogged = true;
    previousState = currentState;
  }
  
  // Para mensajes periódicos, usar control de tiempo
  if (currentTime - lastLogTime >= 1000) // Cada 1 segundo
  {
    Serial.println("LineFollowingMode: Estado periódico...");
    lastLogTime = currentTime;
  }
}
```

#### 2. Usar Flags para Mensajes de Inicio

```cpp
// En el header
bool modeStartedLogged;

// En startMode()
void LineFollowingMode::startMode()
{
  if (!modeStartedLogged)
  {
    Serial.println("LineFollowingMode: Modo iniciado");
    modeStartedLogged = true;
  }
}
```

#### 3. Control de Tiempo para Mensajes Periódicos

```cpp
// Para mensajes que necesitas ver periódicamente pero no constantemente
static unsigned long lastStatusLog = 0;
unsigned long currentTime = millis();

if (currentTime - lastStatusLog >= 1000) // Cada 1 segundo máximo
{
  Serial.println((String) "LineFollowingMode: Estado - " + stateToString(currentState));
  lastStatusLog = currentTime;
}
```

### Buenas Prácticas Generales

1. **Nunca imprimir en bucles sin control:**
   - Usar flags para mensajes de cambio de estado
   - Usar timers para mensajes periódicos
   - Limitar frecuencia de impresión (máximo 1-2 veces por segundo)

2. **Priorizar mensajes importantes:**
   - Errores críticos: siempre imprimir
   - Cambios de estado: imprimir una vez
   - Estado periódico: usar timer (1-5 segundos)
   - Debug detallado: comentar en producción

3. **Alternativas a Serial.println():**
   - Usar `Serial.print()` con `\n` solo al final (más eficiente)
   - Considerar deshabilitar logs en producción
   - Usar niveles de log (ERROR, WARN, INFO, DEBUG)

### Referencias

- Ver implementación completa en: `lib/follow_mode/follow_mode.cpp` (líneas 88-105)
- Comentario sobre el problema en: `lib/sensor_servo/sensor_servo.cpp` (líneas 44-46)
- Implementación de control de logs en: `lib/car_actions/car_actions.cpp` (solo imprime cuando cambia la acción)

## Estados del Modo

### LineFollowingModeState
- `IDLE`: Modo inactivo, esperando detección de línea
- `MOVING_FORWARD`: Avanzando siguiendo la línea
- `TURNING_LEFT`: Girando a la izquierda para seguir la línea
- `TURNING_RIGHT`: Girando a la derecha para seguir la línea

### LineState
Representa el estado de detección de la línea por los sensores:
- `CENTER`: Línea detectada en el centro (010)
- `HARD_LEFT`: Línea detectada a la izquierda (100)
- `HARD_RIGHT`: Línea detectada a la derecha (001)
- `SOFT_LEFT`: Línea detectada en izquierda + centro (110)
- `SOFT_RIGHT`: Línea detectada en centro + derecha (011)
- `ALL_DETECTED`: Línea muy ancha o intersección (111)
- `LOST_LEFT`: Línea perdida, última vez vista a la izquierda (000)
- `LOST_RIGHT`: Línea perdida, última vez vista a la derecha (000)
- `LOST_CENTER`: Línea perdida, última vez vista en centro (000)

## Implementación Pendiente

El modo está actualmente en desarrollo. Ver `TODO` en el código para detalles de implementación.
