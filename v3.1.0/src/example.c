#include <zephyr/kernel.h>
#include "nrf52_pcnt.h"

#define SLEEP_TIME_MS 5000
#define BUTTON_PIN 11

int main() {
        nrfx_err_t errCode = pcntInit(BUTTON_PIN);
        uint32_t count;

        for (;;) {
                pcntClear();
                printk("Sleeping for %d seconds, peripherals will still count pulses even while CPU is asleep\n\n", SLEEP_TIME_MS / 1000);
                k_msleep(SLEEP_TIME_MS);
                count = pcntGetCount();
                printk("%u pulse(s) counted this cycle, resetting counter...\n\n", count);
        }
  return 0;
}
