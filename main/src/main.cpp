#include "System.h"

extern "C" void app_main() {
    System system;
    system.init();
    system.loop();
}
