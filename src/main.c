#include <zephyr/kernel.h>
#include "nrf52_pcnt.h"

#define SLEEP_TIME_MS 5000
#define BUTTON_PIN 11

int main() {
        pcntInit(BUTTON_PIN);

        for (;;) {
                pcntClear();
                printk("Sleeping for %d seconds, peripherals will still count pulses even while CPU is asleep\n", SLEEP_TIME_MS / 1000);
                k_msleep(SLEEP_TIME_MS);
                pcntGetCount();
        }
  return 0;
}
