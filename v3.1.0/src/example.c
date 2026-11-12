#include <zephyr/kernel.h>
#include "nrf52_pcnt.h"

#define MEASURE_TIME_MS 1000
#define SLEEP_TIME_MS 5000 // Must be >= MEASURE_TIME_MS
#define BUTTON_PIN 11

int main(void) {
        nrfx_err_t errCode = pcntInit(BUTTON_PIN, MEASURE_TIME_MS);
        if (errCode != NRFX_SUCCESS) {
                printk("pcntInit failed: 0x%08x\n", errCode);
                return 0;
        }

        uint32_t count;

        for (;;) {
                // Start a new measurement window
                pcntClearCounter();
                pcntClearTimer();
                printk("Sleeping for %d seconds, peripherals will count pulses for %d ms even while CPU is asleep\n\n",
                       SLEEP_TIME_MS / 1000, MEASURE_TIME_MS);
                k_msleep(SLEEP_TIME_MS);

                // Count was latched by hardware when the window elapsed
                count = pcntGetCount();
                printk("%u pulse(s) counted in %d ms, resetting counter...\n\n", count, MEASURE_TIME_MS);
        }
        return 0;
}
