#include "System.h"
#include "esp_system.h"

extern "C" void app_main() {
    {
        System system;
        system.init();
        system.loop();
        system.free();
    }

    esp_restart();
}
