# ESP32 WROVER - Servidor de Cámara Web

## Descripción del Proyecto
Este proyecto implementa un servidor web en un ESP32 WROVER que permite streaming de video en tiempo real desde la cámara integrada. El sistema incluye funcionalidades de control remoto y comunicación bidireccional a través de WebSockets.

## Estructura del Proyecto

### Librerías Principales

#### 1. **esp_camera.h**
- **Propósito**: Librería oficial de Espressif para el control de cámaras OV2640/OV5640
- **Funcionalidades**:
  - Configuración de pines de la cámara
  - Captura de imágenes en diferentes formatos (JPEG, RGB565, etc.)
  - Control de resolución y calidad de imagen
  - Configuración de parámetros como brillo, contraste, saturación
- **Uso en el proyecto**: Manejo completo de la cámara para streaming de video

#### 2. **camera_pins.h**
- **Propósito**: Definición de pines GPIO para el modelo de cámara que tenemos que es el "CAMERA_MODEL_WROVER_KIT"
- **Funcionalidades**:
  - Configuración automática según el modelo de cámara detectado
  - Soporte para múltiples modelos: WROVER_KIT, ESP_EYE, M5STACK, AI_THINKER, etc.
  - Definición de pines para: datos de imagen (Y2-Y9), sincronización (VSYNC, HREF, PCLK), control (XCLK, SIOD, SIOC)
- **Uso en el proyecto**: Configuración automática de pines según el hardware

**Pines de la Cámara OV2640/OV5640 (WROVER_KIT):**

**Pines de Control:**
- **PWDN_GPIO_NUM (-1)**: Pin de Power Down (no usado en WROVER_KIT)
- **RESET_GPIO_NUM (-1)**: Pin de Reset (no usado en WROVER_KIT)
- **XCLK_GPIO_NUM (21)**: Clock externo para la cámara (24MHz)
- **SIOD_GPIO_NUM (26)**: Línea de datos I2C (SDA) para configuración de la cámara
- **SIOC_GPIO_NUM (27)**: Línea de clock I2C (SCL) para configuración de la cámara

**Pines de Datos de Imagen (8 bits paralelos):**
- **Y9_GPIO_NUM (35)**: Bit más significativo (MSB) de datos de imagen
- **Y8_GPIO_NUM (34)**: Bit 7 de datos de imagen
- **Y7_GPIO_NUM (39)**: Bit 6 de datos de imagen
- **Y6_GPIO_NUM (36)**: Bit 5 de datos de imagen
- **Y5_GPIO_NUM (19)**: Bit 4 de datos de imagen
- **Y4_GPIO_NUM (18)**: Bit 3 de datos de imagen
- **Y3_GPIO_NUM (5)**: Bit 2 de datos de imagen
- **Y2_GPIO_NUM (4)**: Bit menos significativo (LSB) de datos de imagen

**Pines de Sincronización:**
- **VSYNC_GPIO_NUM (25)**: Sincronización vertical - indica el inicio de un nuevo frame
- **HREF_GPIO_NUM (23)**: Sincronización horizontal - indica el inicio de una nueva línea
- **PCLK_GPIO_NUM (22)**: Pixel Clock - sincroniza la transferencia de cada pixel

**Flujo de Datos:**
1. **XCLK** proporciona el clock maestro a la cámara
2. **SIOD/SIOC** configuran parámetros de la cámara (resolución, formato, etc.)
3. **VSYNC** indica inicio de frame
4. **HREF** indica inicio de línea
5. **PCLK** sincroniza cada pixel
6. **Y2-Y9** transmiten los datos de imagen en paralelo



#### 3. **WebServer.h**
- **Propósito**: Servidor HTTP para servir páginas web y API REST
- **Funcionalidades**:
  - Servir archivos HTML, CSS, JavaScript
  - Manejo de rutas HTTP (GET, POST)
  - API para control remoto de la cámara
- **Uso en el proyecto**: Interfaz web para visualizar el streaming y controlar la cámara

#### 4. **WebSocketsServer.h**
- **Propósito**: Comunicación bidireccional en tiempo real entre cliente y servidor
- **Funcionalidades**:
  - Streaming de video en tiempo real
  - Envío de comandos de control
  - Notificaciones de estado
- **Uso en el proyecto**: Transmisión de video y comandos de control

#### 5. **WiFi.h**
- **Propósito**: Configuración y manejo de conectividad WiFi
- **Funcionalidades**:
  - Configuración de punto de acceso (AP)
  - Conexión a redes WiFi existentes
  - Manejo de eventos de conexión/desconexión
- **Uso en el proyecto**: Crear red WiFi para conectar dispositivos cliente

### Componentes del Sistema

#### **ESP32Server Class**
- **Propósito**: Clase principal que coordina todos los componentes
- **Funcionalidades**:
  - Inicialización de cámara, WiFi, servidor web y WebSockets
  - Manejo de clientes conectados
  - Control de LED indicador
  - Comunicación UART para debugging

#### **Configuración de Hardware**
- **Cámara**: OV2640/OV5640 (según modelo)
- **LED**: Pin 46 para indicación de estado
- **UART**: Pines 40 (TX) y 3 (RX) para comunicación serial
- **WiFi**: Modo punto de acceso "ESP32CAM" con contraseña "12345678"

## Configuración del Proyecto

### Dependencias (platformio.ini)
```ini
lib_deps =
    WiFi              # Conectividad WiFi
    WebServer         # Servidor HTTP
    Wire              # Comunicación I2C
    SPI               # Comunicación SPI
    esp32-camera      # Librería de cámara
    links2004/WebSockets @ ^2.4.1  # WebSockets
```

### Flags de Compilación
- `BOARD_HAS_PSRAM`: Habilita uso de PSRAM para mejor rendimiento
- `ARDUINO_USB_MODE=1`: Habilita modo USB
- `CONFIG_SPIRAM_CACHE_WORKAROUND`: Optimización para PSRAM

## Uso

1. **Compilación**: `pio run`
2. **Subida**: `pio run --target upload`
3. **Monitoreo**: `pio device monitor`
4. **Conexión**: Conectar al WiFi "ESP32CAM" con contraseña "12345678"
5. **Acceso**: Abrir navegador en `http://192.168.4.1`

## Características Técnicas

- **Resolución**: Configurable (VGA, SVGA, XGA, etc.)
- **Formato**: JPEG para streaming web
- **FPS**: Hasta 30 FPS dependiendo de resolución
- **Clientes**: Múltiples clientes simultáneos
- **Control**: Ajustes de cámara en tiempo real

## Notas de Desarrollo

- El proyecto está configurado para ESP32-S3 con PSRAM
- Soporta múltiples modelos de cámara automáticamente
- Incluye sistema de debugging por UART
- Optimizado para streaming web en tiempo real
