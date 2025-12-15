#include <nrfx_timer.h>
#include <nrfx_ppi.h>
#include <zephyr/kernel.h>

#define TIMER_NUM 0 // Change this if application already uses default timer (0)

static const nrfx_timer_t pcntTimer = NRFX_TIMER_INSTANCE(TIMER_NUM);

// FOR PPI TESTING

static const nrfx_timer_t cntTimer = NRFX_TIMER_INSTANCE(1);
void setupCntTimer() {
        nrfx_timer_config_t cntTimerConfig = NRFX_TIMER_DEFAULT_CONFIG(1000000);
        uint32_t err = nrfx_timer_init(&cntTimer, &cntTimerConfig, NULL);
        if (err != 0) {
                printk("nrfx_timer_init error: %08x", err);
                return;
        }
        uint32_t timer_ticks = nrfx_timer_us_to_ticks(&cntTimer, 1);
        nrfx_timer_extended_compare(&cntTimer, NRF_TIMER_CC_CHANNEL1, timer_ticks, NRF_TIMER_SHORT_COMPARE1_CLEAR_MASK, false);
}

void setupTimer() {
  nrfx_timer_config_t pcntTimerConfig = NRFX_TIMER_DEFAULT_CONFIG(9000000);
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

void clearCounter() {
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

// PPI
static nrf_ppi_channel_t ppiChannel;
void setupPPI() {
        nrfx_ppi_channel_alloc(&ppiChannel);

        nrfx_ppi_channel_assign(ppiChannel,
                nrfx_timer_compare_event_address_get(&cntTimer, NRF_TIMER_CC_CHANNEL1),
                nrfx_timer_task_address_get(&pcntTimer, NRF_TIMER_TASK_COUNT));

        nrfx_ppi_channel_enable(ppiChannel);
}

int main(void) {
  setupTimer();
  setupCntTimer();
  setupPPI();
  nrfx_timer_enable(&cntTimer);
  printk("Finished Setup\n");
  while(1) {
        getCounterVal();
  }
  return 0;
}
