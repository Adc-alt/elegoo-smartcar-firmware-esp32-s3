# JSON compacto ESP32 → Atmega328P

El JSON de comandos que envía el ESP32 al Atmega usa **claves y valores cortos** para caber en el buffer serie de 64 bytes del Arduino (evitar overflow y corrupción).

## Equivalencias de claves

| Clave larga (antigua) | Clave compacta | Descripción        |
|------------------------|----------------|--------------------|
| `servoAngle`           | `sA`           | Ángulo del servo   |
| `ledColor`             | `lC`           | Color del LED      |
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

## Ejemplo de mensaje

**Antes (≈97 caracteres):**
```json
{"servoAngle":90,"ledColor":"YELLOW","motors":{"left":{"action":"freeStop","speed":0},"right":{"action":"freeStop","speed":0}}}
```

**Después (≈52 caracteres):**
```json
{"sA":90,"lC":"Y","m":{"L":{"a":"fS","s":0},"R":{"a":"fS","s":0}}}
```

## Uso en el Atmega

Al deserializar el JSON recibido, leer con las claves compactas:

- `jsonDoc["sA"]` → ángulo servo  
- `jsonDoc["lC"]` → color LED (comparar con "Y", "B", "G", etc.)  
- `jsonDoc["m"]["L"]["a"]` / `jsonDoc["m"]["L"]["s"]` → acción y velocidad motor izquierdo  
- `jsonDoc["m"]["R"]["a"]` / `jsonDoc["m"]["R"]["s"]` → acción y velocidad motor derecho  

Para las acciones, comparar `a` con `"fW"`, `"bW"`, `"tL"`, `"tR"`, `"fS"`, `"fT"`.
