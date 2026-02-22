#pragma once

#include <Arduino.h>

/**
 * Contenido web de la página de cámara (stream + captura).
 * Responsabilidad: solo el HTML; el servidor (ap_handle) lo sirve.
 */
String getCameraPageHtml(void);
