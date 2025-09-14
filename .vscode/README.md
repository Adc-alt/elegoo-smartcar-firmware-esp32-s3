# Configuración de VS Code para Elegoo Smart Car

Esta carpeta contiene todos los archivos de configuración de VS Code necesarios para trabajar con el proyecto Elegoo Smart Car.

## Archivos de Configuración

### 📁 `settings.json`
**¿Qué hace?** Configura cómo VS Code funciona con PlatformIO y el proyecto Elegoo Smart Car.
**¿Se autogenera?** NO - Se crea manualmente y se puede editar.
**¿Es importante?** SÍ - Controla IntelliSense, autocompletado, monitor serial y más.

### 📁 `c_cpp_properties.json`
**¿Qué hace?** Define las rutas de inclusión y configuración del compilador C/C++.
**¿Se autogenera?** SÍ - Se genera automáticamente por PlatformIO.
**¿Es importante?** SÍ - Le dice a VS Code dónde encontrar las librerías del Elegoo Smart Car.

### 📁 `compile_commands.json`
**¿Qué hace?** Contiene los comandos exactos de compilación para cada archivo.
**¿Se autogenera?** SÍ - Se genera automáticamente por PlatformIO.
**¿Es importante?** SÍ - Permite IntelliSense preciso y navegación de código.

### 📁 `launch.json`
**¿Qué hace?** Configura las opciones de debugging para ATMEGA328P y ESP32.
**¿Se autogenera?** SÍ - Se genera automáticamente por PlatformIO.
**¿Es importante?** SÍ - Permite debuggear el código del carro.

### 📁 `extensions.json`
**¿Qué hace?** Recomienda extensiones necesarias y evita conflictos.
**¿Se autogenera?** NO - Se crea manualmente.
**¿Es importante?** SÍ - Evita que VS Code sugiera herramientas incompatibles.

### 📁 `clangd_config.yaml`
**¿Qué hace?** Configura clangd (motor de IntelliSense moderno).
**¿Se autogenera?** NO - Se crea manualmente.
**¿Es importante?** SÍ - Mejora el análisis de código y autocompletado.



## Entornos Disponibles

- **atmega328_car**: Para el carro con ATMEGA328P
- **atmega328_test**: Para pruebas con ATMEGA328P
- **esp32_test**: Para pruebas con ESP32

## Notas Importantes

⚠️ **NO edites archivos autogenerados** (`c_cpp_properties.json`, `compile_commands.json`, `launch.json`)
✅ **SÍ puedes editar** `settings.json`, `extensions.json` y `clangd_config.yaml`

Los archivos autogenerados se recrean automáticamente cuando cambias `platformio.ini`.
