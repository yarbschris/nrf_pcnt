#include "nrf52_pcnt.h"
#include <nrfx_gpiote.h>
#include <nrfx_ppi.h>
#include <nrfx_timer.h>

// Counts the number of pulses on a GPIO pin over a fixed measurement window
// without CPU interrupts

// Change these if the application already uses these peripheral instances.
// TIMER0 is avoided since it is commonly reserved by the radio/RTOS.
// Remember to enable the matching CONFIG_NRFX_TIMERn options in prj.conf
#define COUNTER_TIMER_NUM 2
#define GATE_TIMER_NUM 3
#define GPIOTE_INSTANCE 0

// GLOBAL VARS
static const nrfx_timer_t counter = NRFX_TIMER_INSTANCE(COUNTER_TIMER_NUM);
static const nrfx_timer_t timer = NRFX_TIMER_INSTANCE(GATE_TIMER_NUM);
static const nrfx_gpiote_t gpiote = NRFX_GPIOTE_INSTANCE(GPIOTE_INSTANCE);
static uint8_t inChannel;
static nrf_ppi_channel_t counterChannelPPI;
static nrf_ppi_channel_t captureChannelPPI;
static const nrf_timer_cc_channel_t countChannel = NRF_TIMER_CC_CHANNEL0;
static const nrf_timer_cc_channel_t compareChannel = NRF_TIMER_CC_CHANNEL1;

// COUNTER - This hardware timer will be used as a counter to keep track of
// pulses. In counter mode, it is only incremented when the COUNT task is
// triggered by PPI
static nrfx_err_t setupCounter(void) {
  nrfx_timer_config_t counterConfig = NRFX_TIMER_DEFAULT_CONFIG(1000000);
  counterConfig.mode = NRF_TIMER_MODE_COUNTER;
  counterConfig.bit_width = NRF_TIMER_BIT_WIDTH_32;

  nrfx_err_t err = nrfx_timer_init(&counter, &counterConfig, NULL);
  if (err != NRFX_SUCCESS) {
    return err;
  }

  nrfx_timer_enable(&counter);
  return NRFX_SUCCESS;
}

// TIMER - This hardware timer will be used to make sure the counter measures
// for whatever measureTime is
static nrfx_err_t setupTimer(uint16_t measureTimeMs) {
  nrfx_timer_config_t timerConfig = NRFX_TIMER_DEFAULT_CONFIG(1000000);
  timerConfig.mode = NRF_TIMER_MODE_TIMER;
  timerConfig.bit_width = NRF_TIMER_BIT_WIDTH_32;

  nrfx_err_t err = nrfx_timer_init(&timer, &timerConfig, NULL);
  if (err != NRFX_SUCCESS) {
    return err;
  }

  // Set the timer compare event to trigger on measure time.
  // No CPU interrupt needed — PPI captures the counter autonomously.
  uint32_t measureTimeTicks = nrfx_timer_ms_to_ticks(&timer, measureTimeMs);
  nrfx_timer_compare(&timer, compareChannel, measureTimeTicks, false);

  nrfx_timer_enable(&timer);
  return NRFX_SUCCESS;
}

// GPIOTE - Will trigger an event on each pulse that tells counter to increment
static nrfx_err_t setupGPIOTE(nrfx_gpiote_pin_t pulsePin) {
  nrfx_err_t err;

  // Zephyr's GPIO driver normally initializes GPIOTE already
  if (!nrfx_gpiote_init_check(&gpiote)) {
    err = nrfx_gpiote_init(&gpiote, 0);
    if (err != NRFX_SUCCESS) {
      return err;
    }
  }

  // Allocate GPIOTE Channel
  err = nrfx_gpiote_channel_alloc(&gpiote, &inChannel);
  if (err != NRFX_SUCCESS) {
    return err;
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

  err = nrfx_gpiote_input_configure(&gpiote, pulsePin, &inputConfig);
  if (err != NRFX_SUCCESS) {
    return err;
  }

  nrfx_gpiote_trigger_enable(&gpiote, pulsePin, false);

  return NRFX_SUCCESS;
}

// PPI: GPIOTE in event -> Counter Count Task
static nrfx_err_t setupCounterPPI(nrfx_gpiote_pin_t pulsePin) {
  nrfx_err_t err = nrfx_ppi_channel_alloc(&counterChannelPPI);
  if (err != NRFX_SUCCESS) {
    return err;
  }

  err = nrfx_ppi_channel_assign(
      counterChannelPPI, nrfx_gpiote_in_event_address_get(&gpiote, pulsePin),
      nrfx_timer_task_address_get(&counter, NRF_TIMER_TASK_COUNT));
  if (err != NRFX_SUCCESS) {
    return err;
  }

  return nrfx_ppi_channel_enable(counterChannelPPI);
}

// PPI: Timer Compare Event -> Counter Capture Task
// This will capture the counter value when measureTimeMs has elapsed
static nrfx_err_t setupCapturePPI(void) {
  nrfx_err_t err = nrfx_ppi_channel_alloc(&captureChannelPPI);
  if (err != NRFX_SUCCESS) {
    return err;
  }

  err = nrfx_ppi_channel_assign(
      captureChannelPPI,
      nrfx_timer_compare_event_address_get(&timer, compareChannel),
      nrfx_timer_capture_task_address_get(&counter, countChannel));
  if (err != NRFX_SUCCESS) {
    return err;
  }

  return nrfx_ppi_channel_enable(captureChannelPPI);
}

// Public Functions
nrfx_err_t pcntInit(int pin, uint16_t measureTimeMs) {
  nrfx_gpiote_pin_t pulsePin = (nrfx_gpiote_pin_t)pin;

  nrfx_err_t err = setupCounter();
  if (err != NRFX_SUCCESS) {
    return err;
  }

  err = setupTimer(measureTimeMs);
  if (err != NRFX_SUCCESS) {
    return err;
  }

  err = setupGPIOTE(pulsePin);
  if (err != NRFX_SUCCESS) {
    return err;
  }

  err = setupCapturePPI();
  if (err != NRFX_SUCCESS) {
    return err;
  }

  return setupCounterPPI(pulsePin);
}

uint32_t pcntCaptureAndGetCount(void) {
  return nrfx_timer_capture(&counter, countChannel);
}

uint32_t pcntGetCount(void) {
  return nrfx_timer_capture_get(&counter, countChannel);
}

void pcntClearCounter(void) { nrfx_timer_clear(&counter); }
void pcntClearTimer(void) { nrfx_timer_clear(&timer); }
