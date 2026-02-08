#ifndef WEB_SERVER_HOST_H
#define WEB_SERVER_HOST_H

#include <WebServer.h>
#include <WiFi.h>
#include <functional>

/**
 * Posee el WebServer HTTP. Responsabilidades:
 * - Registrar rutas: / (página con botones), /ping, POST /command.
 * - Arrancar el servidor (init) y atender peticiones (loop).
 * - setCommandCallback para notificar comandos recibidos.
 * - getServer() para que otros módulos (streaming, etc.) registren rutas.
 */
class WebServerHost
{
public:
  void init(void);
  void loop(void);

  void setCommandCallback(std::function<void(const char*, int)> cb);

  /** Puntero al servidor para que otros módulos registren rutas. */
  WebServer* getServer(void);

private:
  WebServer server;

  void setup_routes(void);
  void handle_root(void);
  void handle_ping(void);
  void handle_command(void);

  std::function<void(const char*, int)> commandCallback;
};

#endif
