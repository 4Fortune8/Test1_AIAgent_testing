#include "control/control_loop.h"

extern "C" void app_main(void) {
    start_control_loop();

    while (true) {
        // Core 1 idle / CAN later
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
