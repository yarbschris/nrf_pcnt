#include <nrfx_timer.h>
#include <zephyr/kernel.h>

#define TIMER_NUM 0 // Change this if application already uses default timer (0)

static const nrfx_timer_t pcntTimer = NRFX_TIMER_INSTANCE(TIMER_NUM);

void setupTimer() {
  nrfx_timer_config_t pcntTimerConfig = NRFX_TIMER_DEFAULT_CONFIG(1000000);
  pcntTimerConfig.mode = NRF_TIMER_MODE_COUNTER;

  nrfx_err_t errCode = nrfx_timer_init(&pcntTimer, &pcntTimerConfig, NULL);
  if (errCode != NRFX_SUCCESS) {
    printk("Init timer failed\n");
    printk("Error Code: %d\n", errCode);
  } else {
    printk("timer initialized\n");
  }
  nrfx_timer_enable(&pcntTimer);
  printk("timer enabled\n");
}

// Will be replaced with a timer COUNT event triggered by gpio pin further along
// This function is for test purposes
void incrementCounter() {
  nrfx_timer_increment(&pcntTimer);
}

uint32_t clearCounter() {
  printk("Clearing pcntTimer\n");
  nrfx_timer_clear(&pcntTimer);
}

uint32_t getCounterVal() {
  nrfx_timer_capture(&pcntTimer, NRF_TIMER_CC_CHANNEL0);
  uint32_t val = nrfx_timer_capture_get(&pcntTimer, NRF_TIMER_CC_CHANNEL0);
  printk("Current pcntTimer value: %u\n", val);
  return val;
}

uint32_t getCounterCountAddress() {
        printk("Getting counter COUNT address...\n");
        uint32_t addr = nrfx_timer_task_address_get(&pcntTimer, NRF_TIMER_TASK_COUNT);
        printk("Address: %u", addr);
        return addr;
}

int main(void) {
  printk("Starting...\n");
  k_msleep(100);
  setupTimer();
  printk("Finished Setup\n");
  getCounterCountAddress();
  return 0;
}
