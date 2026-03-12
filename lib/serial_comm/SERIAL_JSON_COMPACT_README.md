# JSON compacto ESP32 → Atmega328P

El JSON de comandos que envía el ESP32 al Atmega usa **claves y valores cortos** para caber en el buffer serie de 64 bytes del Arduino (evitar overflow y corrupción).

## Equivalencias de claves

| Clave larga (antigua) | Clave compacta | Descripción        |
|------------------------|----------------|--------------------|
| `servoAngle`           | `sA`           | Ángulo del servo   |
| `ledColor`             | `lC`           | Color del LED      |
<<<<<<< HEAD
| `mode`                 | `Md`           | Modo actual        |
=======
| `mode`                 | `Md`           | Modo actual (0–6)  |
>>>>>>> d8fb2749111071840f58408a7ffeb354b672290b
| `motors`               | `m`            | Objeto motores     |
| `left`                 | `L`            | Motor izquierdo    |
| `right`                | `R`            | Motor derecho      |
| `action`               | `a`            | Acción del motor   |
| `speed`                | `s`            | Velocidad (0–255)  |

## Equivalencias de valores: acción (`a`)

| Valor largo   | Código compacto |
|---------------|------------------|
| `forward`     | `fW`             |
| `backward`    | `bW`             |
| `turnLeft`    | `tL`             |
| `turnRight`   | `tR`             |
| `freeStop`    | `fS`             |
| `forceStop`   | `fT`             |

## Equivalencias de valores: LED (`lC`)

| Valor largo | Código compacto |
|-------------|------------------|
| `YELLOW`    | `Y`              |
| `BLUE`      | `B`              |
| `GREEN`     | `G`              |
| `PURPLE`    | `P`              |
| `WHITE`     | `W`              |
| `SALMON`    | `S`              |
| `CYAN`      | `C`              |
| (cualquier otro) | `Y` (por defecto) |

## Valores del modo (`Md`)

| Valor | Modo                    |
|-------|-------------------------|
| 0     | IR_MODE                 |
| 1     | OBSTACLE_AVOIDANCE_MODE  |
| 2     | FOLLOW_MODE             |
| 3     | LINE_FOLLOWING_MODE      |
| 4     | RC_MODE                 |
| 5     | BALL_FOLLOW_MODE        |
| 6     | IDLE (no hacer nada)    |

## Ejemplo de mensaje

**Antes (≈97 caracteres):**
```json
{"servoAngle":90,"ledColor":"YELLOW","motors":{"left":{"action":"freeStop","speed":0},"right":{"action":"freeStop","speed":0}}}
```

<<<<<<< HEAD
**Después (≈55 caracteres):**
=======
**Después (≈58 caracteres):**
>>>>>>> d8fb2749111071840f58408a7ffeb354b672290b
```json
{"sA":90,"lC":"Y","Md":6,"m":{"L":{"a":"fS","s":0},"R":{"a":"fS","s":0}}}
```

## Equivalencias de valores: modo (`Md`)

`Md` es un número (0–6), según el orden del enum `CarMode`:

| Número | Modo (CarMode)              |
|--------|-----------------------------|
| 0      | `IR_MODE`                   |
| 1      | `OBSTACLE_AVOIDANCE_MODE`   |
| 2      | `FOLLOW_MODE`               |
| 3      | `LINE_FOLLOWING_MODE`       |
| 4      | `RC_MODE`                   |
| 5      | `BALL_FOLLOW_MODE`          |
| 6      | `IDLE`                      |

## Uso en el Atmega

Al deserializar el JSON recibido, leer con las claves compactas:

- `jsonDoc["sA"]` → ángulo servo  
- `jsonDoc["lC"]` → color LED (comparar con "Y", "B", "G", etc.)  
<<<<<<< HEAD
- `jsonDoc["Md"]` → modo actual (0=IR, 1=Obstacle, 2=Follow, 3=Line, 4=RC, 5=Ball, 6=IDLE)  
=======
- `jsonDoc["Md"]` → modo actual (0–6)  
>>>>>>> d8fb2749111071840f58408a7ffeb354b672290b
- `jsonDoc["m"]["L"]["a"]` / `jsonDoc["m"]["L"]["s"]` → acción y velocidad motor izquierdo  
- `jsonDoc["m"]["R"]["a"]` / `jsonDoc["m"]["R"]["s"]` → acción y velocidad motor derecho  

Para las acciones, comparar `a` con `"fW"`, `"bW"`, `"tL"`, `"tR"`, `"fS"`, `"fT"`.
