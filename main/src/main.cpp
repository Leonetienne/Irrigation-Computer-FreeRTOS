#include "System.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" void app_main() {
    System system;
    system.Init();

    while (true) {
        system.Update();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
