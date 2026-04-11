#ifndef WEB_SERVER_HOST_H
#define WEB_SERVER_HOST_H

#include <WebServer.h>
#include <functional>

/**
 * Posee el WebServer HTTP. Responsabilidades:
 * - Registrar rutas: / (página con botones), /ping, POST /command, POST /motors.
 * - Arrancar el servidor (init) y atender peticiones (loop).
 * - setCommandCallback para notificar comandos simples (action + speed).
 * - setDifferentialCallback para notificar comandos diferenciales (left/right action + speed).
 * - getServer() para que otros módulos registren rutas adicionales en el mismo servidor.
 *
 * La ruta GET /streaming (MJPEG de cámara) no se declara aquí: la registra la clase Streaming
 * vía getServer(), para no mezclar la lógica HTTP genérica con pines/esp_camera. No fusionar
 * Streaming dentro de WebServerHost: son responsabilidades distintas (mismo WebServer, módulos separados).
 */
class WebServerHost
{
public:
  void init(void);
  void loop(void);

  void setCommandCallback(std::function<void(const char*, int)> cb);
  void setDifferentialCallback(std::function<void(const char*, uint8_t, const char*, uint8_t)> cb);

  /**
   * Puntero al WebServer compartido. Ej.: Streaming registra GET /streaming aquí.
   * Ver comentario de la clase arriba.
   */
  WebServer* getServer(void);

private:
  WebServer server;

  void setupRoutes(void);
  void handleRoot(void);
  void handlePing(void);
  void handleCommand(void);
  void handleDifferentialCommand(void);

  std::function<void(const char*, int)> commandCallback;
  std::function<void(const char*, uint8_t, const char*, uint8_t)> differentialCallback;
};

#endif
