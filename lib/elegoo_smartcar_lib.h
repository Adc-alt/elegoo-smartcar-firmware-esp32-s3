/*
 * ============================================================================
 * ELEGOO SMART CAR ESP32-S3 - CONFIGURACIÓN DE PINES
 * ============================================================================
 * Este archivo define todos los pines utilizados en el ELEGOO Smart Car
 * con ESP32-S3 para la cámara y comunicación UART
 */

// === CONFIGURACIÓN UART2 PARA COMUNICACIÓN SERIAL ===
// Pines para comunicación serial con otros módulos del smart car
#define UART2_TX 40  // Pin de transmisión UART2 (envía datos)
#define UART2_RX 3   // Pin de recepción UART2 (recibe datos)
// Configuración alternativa comentada (puede usarse si hay conflictos)
// #define UART2_TX 4
// #define UART2_RX 33

// === CONFIGURACIÓN DE PINES PARA CÁMARA ESP32-S3 ===
// Pines de control de potencia de la cámara
#define PWDN_GPIO_NUM     -1  // Power Down (no usado en este modelo)
#define RESET_GPIO_NUM    -1  // Reset (no usado en este modelo)

// === PINES DE RELOJ Y SINCRONIZACIÓN ===
#define XCLK_GPIO_NUM     15  // Reloj maestro externo (10MHz) - sincroniza toda la cámara
#define PCLK_GPIO_NUM     13  // Reloj de píxel - marca cada píxel de datos
#define VSYNC_GPIO_NUM    6   // Sincronización vertical - indica inicio de cada frame
#define HREF_GPIO_NUM     7   // Sincronización horizontal - indica inicio de cada línea

// === PINES DE COMUNICACIÓN I2C/SCCB ===
// Para configurar parámetros de la cámara (resolución, brillo, contraste, etc.)
#define SIOD_GPIO_NUM     4   // Datos SCCB (Serial Camera Control Bus) - SDA
#define SIOC_GPIO_NUM     5   // Reloj SCCB - SCL

// === PINES DE DATOS PARALELOS (8 bits) ===
// Estos pines transmiten los datos de imagen pixel por pixel en paralelo
#define Y2_GPIO_NUM       11  // Bit 0 de datos (LSB - bit menos significativo)
#define Y3_GPIO_NUM       9   // Bit 1 de datos
#define Y4_GPIO_NUM       8   // Bit 2 de datos
#define Y5_GPIO_NUM       10  // Bit 3 de datos
#define Y6_GPIO_NUM       12  // Bit 4 de datos
#define Y7_GPIO_NUM       18  // Bit 5 de datos
#define Y8_GPIO_NUM       17  // Bit 6 de datos
#define Y9_GPIO_NUM       16  // Bit 7 de datos (MSB - bit más significativo)

// === PINES UART ADICIONALES ===
// Pines alternativos para comunicación serial (duplicados de UART2)
#define UART_TX_PIN       40  // Pin de transmisión UART (mismo que UART2_TX)
#define UART_RX_PIN       3   // Pin de recepción UART (mismo que UART2_RX)
#define LED_PIN 2  // LED integrado del ESP32-S3