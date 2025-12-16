#include "nrf52_pcnt.h"
#include <nrfx_timer.h>
#include <nrfx_ppi.h>
#include <nrfx_gpiote.h>
#include <zephyr/kernel.h>

// Counts the number of pulses on a GPIO pin without CPU interrupts

#define TIMER_NUM 0 // Change this if application already uses default timer (0)
#define GPIOTE_INSTANCE 0

// Timer - In counter mode, will only be incremented when COUNT task is triggered by PPI. Also functions to get counter value and clear value
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

// GPIOTE - Will trigger an event that tells timer to increment
nrfx_gpiote_t pulseInstance = NRFX_GPIOTE_INSTANCE(GPIOTE_INSTANCE);
uint8_t inChannel;

void setupGPIOTE(nrfx_gpiote_pin_t pulsePin) {
        nrfx_err_t errCode;
        // Initialize GPIOTE
        /* According to error code, this is already done, so comment out for right now
        errCode = nrfx_gpiote_init(&pulseInstance, 0);
        if (errCode != NRFX_SUCCESS) {
                printk("Error during gpiote init\n");
                printk("Error code: %d\n", errCode);
        }
        */

        // Allocate GPIOTE Channel
        errCode = nrfx_gpiote_channel_alloc(&pulseInstance, &inChannel);
        if (errCode != NRFX_SUCCESS) {
                printk("Error during gpiote alloc\n");
                printk("Error code: %d\n", errCode);
        }
        
        // Configure pulse pin to generate an event on high to low transition
        static const nrf_gpio_pin_pull_t pullConfig = NRF_GPIO_PIN_PULLUP;
        nrfx_gpiote_trigger_config_t triggerConfig = {
                .trigger = NRFX_GPIOTE_TRIGGER_HITOLO,
                .p_in_channel = &inChannel,
        };

        nrfx_gpiote_input_pin_config_t inputConfig = {
                .p_pull_config = &pullConfig,
                .p_trigger_config = &triggerConfig,
        };

        errCode = nrfx_gpiote_input_configure(&pulseInstance, pulsePin, &inputConfig);
        if (errCode != NRFX_SUCCESS) {
                printk("Error during gpiote config\n");
                printk("Error code: %d\n", errCode);
        }

        nrfx_gpiote_trigger_enable(&pulseInstance, pulsePin, false);
}

// PPI - Pulse event on GPIO pin -> COUNT task in timer
static nrf_ppi_channel_t ppiChannel;
void setupPPI(nrfx_gpiote_pin_t pulsePin) {
        nrfx_ppi_channel_alloc(&ppiChannel);

        nrfx_ppi_channel_assign(ppiChannel,
                nrfx_gpiote_in_event_address_get(&pulseInstance, pulsePin),
                nrfx_timer_task_address_get(&pcntTimer, NRF_TIMER_TASK_COUNT));

        nrfx_ppi_channel_enable(ppiChannel);
}

void pcntInit(int pulsePin) {
    nrfx_gpiote_pin_t pin = pulsePin;
    setupTimer();
    setupGPIOTE(pin);
    setupPPI(pin);
}

void pcntClear() {
  printk("Clearing pcntTimer\n");
  nrfx_timer_clear(&pcntTimer);
}

uint32_t pcntGetCount() {
  nrfx_timer_capture(&pcntTimer, NRF_TIMER_CC_CHANNEL0);
  uint32_t val = nrfx_timer_capture_get(&pcntTimer, NRF_TIMER_CC_CHANNEL0);
  printk("Current pcntTimer value: %u\n", val);
  return val;
}