# 📖 Explicación del Flujo de Trabajo - ModeManager

## 🔄 Flujo Completo Paso a Paso

### **PASO 1: Se llama a `updateStates()` cada ciclo**

```
Loop principal → ModeManager.updateStates(inputData, outputData)
```

### **PASO 2: Detecta si presionaste el botón (flanco de subida)**

```
¿Botón estaba suelto ANTES (swPressedPrevious = false)
  Y ahora está presionado (inputData.swPressed = true)?

  SÍ → ¡DETECTÓ PULSACIÓN! → Incrementa modeCounter
  NO → Sigue con el modo actual
```

### **PASO 3: Si detectó pulsación, cambia el modo**

```
modeCounter se incrementa (0→1→0→1...)
  ↓
getModeFromCounter() convierte el número a modo:
  - 0 → CarMode::IDLE
  - 1 → CarMode::IR_MODE
  - 2+ → Vuelve a IDLE (ciclo circular)
  ↓
Guarda el modo anterior:
  previousMode = currentMode
  ↓
Actualiza el modo actual:
  currentMode = nuevo modo
```

### **PASO 4: Actualiza el LED según el modo**

```
Si currentMode == IR_MODE:
  outputData.ledColor = "BLUE" (azul)

Si currentMode == IDLE:
  outputData.ledColor = "YELLOW" (amarillo)
```

### **PASO 5: Ejecuta la lógica del modo activo**

```
Si currentMode == IR_MODE:
  → Crea instancia de IrMode
  → Llama a irMode.update(inputData, outputData)
  → IrMode procesa comandos IR y modifica outputData

Si currentMode == IDLE:
  → outputData.leftAction = "freeStop", outputData.rightAction = "freeStop" (parar)
  → outputData.leftSpeed = 0, outputData.rightSpeed = 0 (velocidad 0)
  → outputData.servoAngle = 90 (centro)
```

### **PASO 6: Guarda el estado del botón para la próxima vez**

```
swPressedPrevious = inputData.swPressed
(Así la próxima vez sabrá si había un cambio)
```

---

## 📊 Diagrama Visual del Flujo

```
┌─────────────────────────────────────┐
│   Loop Principal (cada milisegundo) │
└──────────────┬──────────────────────┘
               │
               ↓
┌──────────────────────────────────────┐
│  ModeManager.updateStates()          │
│  • Recibe: inputData (sensores)      │
│  • Modifica: outputData (acciones)   │
└──────────────┬───────────────────────┘
               │
               ↓
      ¿Botón presionado ahora?
      (Y antes estaba suelto?)
               │
        ┌──────┴──────┐
        │ SÍ          │ NO
        ↓             ↓
   Incrementa      Sigue con
   modeCounter     modo actual
        │
        ↓
   Determina nuevo modo
   (0→IDLE, 1→IR_MODE)
        │
        ↓
   Actualiza currentMode
   y previousMode
        │
        ↓
   Asigna color LED
   (IDLE=YELLOW, IR_MODE=BLUE)
        │
        ↓
   Ejecuta lógica del modo:
   ┌────────────────────┐
   │ IR_MODE            │
   │ → IrMode.update()  │
   │ → Procesa IR       │
   └────────────────────┘
   ┌────────────────────┐
   │ IDLE               │
   │ → Para el coche    │
   │ → speed = 0        │
   └────────────────────┘
        │
        ↓
   Guarda estado del botón
   para la próxima iteración
        │
        ↓
   Retorna (outputData modificado)
```

---

## 🎯 Ejemplo Práctico

### **Situación Inicial:**

- `modeCounter = 0`
- `currentMode = IDLE`
- `swPressedPrevious = false`
- Botón NO presionado

### **Acción 1: Presionas el botón**

```
Ciclo 1:
  inputData.swPressed = true
  swPressedPrevious = false
  → Detecta flanco de subida ✓
  → modeCounter = 1
  → currentMode = IR_MODE
  → outputData.ledColor = "BLUE"
  → Ejecuta IrMode.update()
```

### **Acción 2: Sueltas el botón**

```
Ciclo 2:
  inputData.swPressed = false
  swPressedPrevious = true
  → NO detecta flanco (está soltando, no presionando)
  → Sigue en IR_MODE
  → outputData.ledColor = "BLUE"
  → Ejecuta IrMode.update()
```

### **Acción 3: Presionas el botón de nuevo**

```
Ciclo 3:
  inputData.swPressed = true
  swPressedPrevious = false
  → Detecta flanco de subida ✓
  → modeCounter = 2
  → (como >= 2, resetea a 0)
  → modeCounter = 0
  → currentMode = IDLE
  → outputData.ledColor = "YELLOW"
  → Para el coche (speed = 0)
```

---

## 🔑 Conceptos Clave

1. **Flanco de subida**: Solo cambia de modo cuando PRESIONAS el botón, no cuando lo mantienes presionado.

2. **Ciclo circular**: IDLE → IR_MODE → IDLE → IR_MODE...

3. **Polimorfismo**: Cada modo (IrMode, etc.) hereda de `Mode` y tiene su propia lógica en `update()`.

4. **Separación de responsabilidades**:
   - `ModeManager`: Gestiona qué modo está activo
   - `Mode` (IrMode, etc.): Ejecuta la lógica específica del modo

---

## 💡 ¿Por qué funciona así?

- **Detectar flanco**: Evita cambiar de modo múltiples veces si mantienes el botón presionado
- **Contador circular**: Permite agregar más modos fácilmente en el futuro
- **Modo base abstracto**: Facilita agregar nuevos modos sin modificar ModeManager
