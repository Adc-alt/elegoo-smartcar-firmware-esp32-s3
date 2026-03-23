# Streaming MJPEG (`lib/web/streaming`)

Este módulo inicializa la cámara (`esp_camera`) y expone un stream **MJPEG** por HTTP (`multipart/x-mixed-replace`). La **fluidez** (FPS aparente) y la **calidad de imagen / lectura de color** dependen del **encoder JPEG**, de la **resolución** y, sobre todo, del **ancho de banda Wi‑Fi**.

**English:** MJPEG over HTTP; smoothness and color fidelity are limited by JPEG size, resolution, and Wi‑Fi throughput.

---

## Endpoint

| Ruta | Descripción |
|------|-------------|
| `/streaming` | Stream MJPEG (JPEG por frame). El acceso puede estar condicionado por la app (p. ej. solo en cierto modo). |

---

## 1. Parámetros que influyen en fluidez, FPS y latencia

*Fluidez* aquí = cuántos fotogramas por segundo llegan al navegador sin tirones. *Latencia* = retraso entre la escena real y lo que ves.

| Parámetro | EN | Efecto típico |
|-----------|----|----------------|
| **`frame_size`** (`FRAMESIZE_*`) | Output resolution (e.g. QVGA, VGA, HD). | **↑ resolución** → JPEG más grande → **menos FPS**, más **latencia** si el Wi‑Fi no da más. **Mayor impacto** en fluidez. |
| **`jpeg_quality`** (0–63) | JPEG quality in `esp_camera`: **lower number = higher quality** (more bytes). | **↓ número (mejor calidad)** → más bytes/frame → **menos FPS**. **↑ número (peor calidad)** → más FPS, peor detalle y más artefactos en bordes/color. |
| **Wi‑Fi** | Signal, distance, interference. | Misma configuración puede ir fluida o a tirones; suele ser el **cuello de botella** real. |
| **`fb_count`** | Number of frame buffers (often 2). | Memoria y pipeline; impacto **menor** que resolución + JPEG + red. |
| **`grab_mode`** | `CAMERA_GRAB_WHEN_EMPTY` vs `CAMERA_GRAB_LATEST`. | Con **`LATEST`**, bajo carga suele entregarse el **frame más reciente** (menos cola de frames viejos). Mejora **sensación en tiempo real**; no aumenta el FPS máximo teórico del sensor. |
| **`fb_location`** | `CAMERA_FB_IN_PSRAM` vs DRAM. | Con **PSRAM** se pueden usar **resoluciones altas** sin quedarse sin RAM; no elimina el límite del Wi‑Fi. |
| **`xclk_freq_hz`** | Sensor clock (Hz). | Ajuste fino del hardware; **no** es el primer control frente a resolución/JPEG/red para “más FPS”. |

---

## 2. Parámetros que afectan cómo se ve la imagen y la lectura de color (p. ej. verde)

| Ajuste | EN | Efecto visual | Efecto en color / detección |
|--------|----|---------------|------------------------------|
| **`jpeg_quality` (bajo = buena calidad)** | High JPEG quality. | Menos bloques, bordes más limpios. | **Mejor** para apreciar un color estable y umbralizar: menos *bleeding* entre regiones. |
| **`jpeg_quality` (alto = mala calidad)** | Low JPEG quality. | Más bloques y manchas. | **Peor**: el color se **mezcla** en los bordes; el verde se distingue peor. |
| **`frame_size`** | Resolution. | Más píxeles = más detalle. | Mejor **muestreo** del objeto; si el stream va a tirones, puede parecer que el tono **varía** más entre frames (exposición + compresión). |
| **`set_saturation` / `set_contrast` / `set_brightness`** | ISP tone. | Más/menos “punch” y brillo. | Cambian **cómo se ve el verde**; pueden ayudar a separar del fondo o estropear el rango si se pasan. |
| **`set_sharpness`** | Sharpening (−2…2 típ.). | Bordes más/nitidez. | Puede ayudar a ver contornos; **demasiado** + JPEG agresivo puede acentuar **artefactos**. |
| **`set_whitebal` + `set_awb_gain`** | Auto white balance. | Colores agradables en distinta luz. | **Ventaja:** se ve bien en general. **Inconveniente:** el matiz puede **cambiar** entre frames → verde menos estable para reglas HSV muy estrictas. |
| **`set_exposure_ctrl` + `set_gain_ctrl`** | Auto exposure / gain. | Evita imagen muy oscura o quemada. | Similar: **estabilidad vs variación** frame a frame bajo luz cambiante. |

---

## 3. Valores actuales en código (referencia)

> Actualiza esta tabla si cambias `streaming.cpp`.

| Campo | Valor actual | Notas |
|-------|--------------|--------|
| `pixel_format` | `PIXFORMAT_JPEG` | Salida JPEG para MJPEG. |
| `frame_size` | `FRAMESIZE_HD` (1280×720) | Alternativas habituales: `FRAMESIZE_SVGA` (800×600), `FRAMESIZE_VGA` (640×480). Si `esp_camera_init` falla, bajar resolución. |
| `jpeg_quality` | `2` | 0–63: **menor = mejor calidad**. Valores muy bajos = JPEGs muy pesados → menos FPS vía Wi‑Fi. |
| `fb_count` | `2` | Doble buffer. |
| `fb_location` | `CAMERA_FB_IN_PSRAM` | Requiere PSRAM habilitada en el *build* (p. ej. `BOARD_HAS_PSRAM`). |
| `grab_mode` | `CAMERA_GRAB_LATEST` | Prioriza último frame disponible bajo carga. |
| `xclk_freq_hz` | `20_000_000` | 20 MHz. |
| Sensor | `set_vflip(1)` | Orientación. |
| `set_whitebal` / `set_awb_gain` / `set_exposure_ctrl` / `set_gain_ctrl` | `1` | Automatismos ON. |
| `brightness` / `contrast` / `saturation` | `0` / `0` / `0` | Neutros. |
| `sharpness` | `2` | Más nitidez en bordes. |

---

## 4. Cómo tunear: más fluidez sin perder el verde de vista

Orden recomendado:

1. **Bajar `frame_size`** (p. ej. HD → **SVGA** o **VGA**) si necesitas **más FPS**. Suele ser el cambio con **más impacto** en fluidez y suele respetar mejor el color que recortar JPEG al extremo.
2. Si aún falta fluidez, **subir el número** de `jpeg_quality` (peor calidad JPEG = **más alto** en 0–63), **poco a poco** (p. ej. 4 → 8) y comprobar cómo se ve el verde en tu escena.
3. Mantener **`CAMERA_GRAB_LATEST`** si el cuello de botella es la red (menos sensación de “retraso acumulado”).
4. Ajustes finos de **saturación/contraste** solo después, de un paso en el rango típico −2…2.

**English:** Prefer **downscaling resolution** first for higher FPS; only then **raise** `jpeg_quality` number (worse JPEG) in small steps. Watch green edges and threshold behavior.

---

## 5. Referencia API

- `esp_camera` / `camera_config_t`: ver `esp_camera.h` en el SDK de Arduino-ESP32 (ESP-IDF).
- `jpeg_quality`: documentado como **0–63, lower = higher quality**.

---

## Archivos

| Archivo | Rol |
|---------|-----|
| `streaming.cpp` / `streaming.h` | Inicialización de cámara y handler del stream MJPEG. |
