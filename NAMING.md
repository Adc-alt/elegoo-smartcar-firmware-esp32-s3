# Naming

Convención de nombres del firmware (C++ / Arduino). _English: same rules apply project-wide._

Ámbito: código propio en `lib/` y `src/`. **Claves JSON** del protocolo con el ATmega, **macros** de hardware y APIs de terceros pueden seguir otro criterio cuando sea intencional.

## Reglas

- **Clases, structs, namespaces, nombre de `enum class`, alias de tipo** → **PascalCase** (`ModeManager`, `CompactEncode`, `CarMode`).
- **Funciones y métodos** → **camelCase** (`updateStates`, `readJsonBySerial`).
- **Variables locales, parámetros, miembros de instancia** (público o privado) → **camelCase**, sin sufijo `_` (`outputData`, `currentMode`).
- **Constantes de compilación** (`constexpr`, `static constexpr`, `inline constexpr` en archivo o clase) → **kPascalCase** (`kSpeed`, `kSendIntervalMs`).
- **`const` solo en runtime** (inmutable tras init, no `constexpr`) → **camelCase** como cualquier variable (p. ej. referencia ya asignada).
- **Macros `#define` e include guards** → **SCREAMING_SNAKE_CASE** (`UART2_TX`, `WEB_SERVER_HOST_H`).
- **Enumeradores de `enum class`** → **SCREAMING_SNAKE_CASE**, alineado con el repo (`IR_MODE`, `BALL_FOLLOW_MODE`).

## Naming de carpetas y archivos

_English: use `snake_case` for directories and `.h` / `.cpp` names; filenames usually mirror the module folder._

- **Carpetas:** `snake_case`.
- **Archivos (`.h` / `.cpp`):** `snake_case`, normalmente el mismo nombre que la carpeta o el módulo (p. ej. `wifi_ap_manager/wifi_ap_manager.h`).

## Miembros privados

Misma regla: **camelCase, sin `_` final**, salvo decisión explícita del equipo.

## `constexpr` vs `const` (breve)

- **`constexpr`**: valor fijo en **compilación** → `static constexpr` / `constexpr` en archivo y nombre **kPascalCase** si es constante de diseño.
- **`const`**: no cambia tras init, valor **solo en runtime** → sin `k`; **camelCase** habitual.

**Excepción:** `static` **función** con retorno `const char*` (p. ej. `ledColorForMode`) no cuenta como constante `k`.

## Literales

Extraer a **`constexpr` / `static constexpr`** con **kPascalCase** en lugar de números mágicos.
