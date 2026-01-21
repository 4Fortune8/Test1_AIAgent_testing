#include "control_loop.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

static const uint32_t CONTROL_HZ = 2000;
static const uint32_t CONTROL_PERIOD_US = 1000000 / CONTROL_HZ;

static TaskHandle_t control_task_handle = nullptr;

void control_task(void* arg) {
    uint64_t last_time = esp_timer_get_time();

    while (true) {
        uint64_t now = esp_timer_get_time();
        uint64_t elapsed = now - last_time;

        if (elapsed >= CONTROL_PERIOD_US) {
            last_time += CONTROL_PERIOD_US;

            // === REAL-TIME CONTROL LOOP (EMPTY) ===
            // read encoder delta
            // compute velocity
            // PID
            // accel limiting
            // output PWM
        }

        // Yield without blocking determinism
        taskYIELD();
    }
}

void start_control_loop() {
    xTaskCreatePinnedToCore(
        control_task,
        "control_loop",
        4096,
        nullptr,
        configMAX_PRIORITIES - 1,
        &control_task_handle,
        0   // Core 0 = real-time
    );
}
