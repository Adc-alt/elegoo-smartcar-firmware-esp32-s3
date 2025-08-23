#include <Arduino.h>
#include "ESP32Server.h"
// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>

ESP32Server server;

void serverTask(void *pvParameters) {
    server.begin();
    while(1) {
        server.handleClient();
        vTaskDelay(1);
    }
}

void setup() {
    xTaskCreatePinnedToCore(
        serverTask,
        "WebServer",
        8192,
        NULL,
        1,
        NULL,
        0
    );
}

void loop() {
    vTaskDelay(1000);
}
