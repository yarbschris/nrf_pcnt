#include "nrf52_pcnt.h"
#include <nrfx_gpiote.h>
#include <nrfx_ppi.h>
#include <nrfx_timer.h>

// Counts the number of pulses on a GPIO pin without CPU interrupts

#define TIMER_NUM 0 // Change this if application already uses default timer (0)
#define GPIOTE_INSTANCE 0

// Timer - In counter mode, will only be incremented when COUNT task is
// triggered by PPI. Also functions to get counter value and clear value
static const nrfx_timer_t pcntTimer = NRFX_TIMER_INSTANCE(TIMER_NUM);

nrfx_err_t setupTimer()
{
  nrfx_timer_config_t pcntTimerConfig = NRFX_TIMER_DEFAULT_CONFIG(1000000);
  pcntTimerConfig.mode = NRF_TIMER_MODE_COUNTER;

  nrfx_err_t errCode = nrfx_timer_init(&pcntTimer, &pcntTimerConfig, NULL);
  if (errCode != NRFX_SUCCESS)
  {
    return errCode;
  }
  nrfx_timer_enable(&pcntTimer);

  return errCode;
}

// GPIOTE - Will trigger an event that tells timer to increment
nrfx_gpiote_t pulseInstance = NRFX_GPIOTE_INSTANCE(GPIOTE_INSTANCE);
uint8_t inChannel;

nrfx_err_t setupGPIOTE(nrfx_gpiote_pin_t pulsePin)
{
  nrfx_err_t errCode;
  // Initialize GPIOTE
  /* According to error code, this is already done, so comment out for right now
  errCode = nrfx_gpiote_init(&pulseInstance, 0);
  if (errCode != NRFX_SUCCESS) {
          return errCode;
  }
  */

  // Allocate GPIOTE Channel
  errCode = nrfx_gpiote_channel_alloc(&pulseInstance, &inChannel);
  if (errCode != NRFX_SUCCESS)
  {
    return errCode;
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
  if (errCode != NRFX_SUCCESS)
  {
    return errCode;
  }

  nrfx_gpiote_trigger_enable(&pulseInstance, pulsePin, false);

  return errCode;
}

// PPI - Pulse event on GPIO pin -> COUNT task in timer
static nrf_ppi_channel_t ppiChannel;
nrfx_err_t setupPPI(nrfx_gpiote_pin_t pulsePin)
{
  nrfx_err_t errCode;

  nrfx_ppi_channel_alloc(&ppiChannel);

  errCode = nrfx_ppi_channel_assign(
      ppiChannel, nrfx_gpiote_in_event_address_get(&pulseInstance, pulsePin),
      nrfx_timer_task_address_get(&pcntTimer, NRF_TIMER_TASK_COUNT));
  if (errCode != NRFX_SUCCESS)
  {
    return errCode;
  }

  nrfx_ppi_channel_enable(ppiChannel);

  return errCode;
}

nrfx_err_t pcntInit(int pulsePin)
{
  nrfx_err_t errCode;

  nrfx_gpiote_pin_t pin = pulsePin;
  errCode = setupTimer();
  if (errCode != NRFX_SUCCESS)
  {
    return errCode;
  }
  errCode = setupGPIOTE(pin);
  if (errCode != NRFX_SUCCESS)
  {
    return errCode;
  }

  errCode = setupPPI(pin);
  return errCode;
}

void pcntClear()
{
  nrfx_timer_clear(&pcntTimer);
}

uint32_t pcntGetCount()
{
  nrfx_timer_capture(&pcntTimer, NRF_TIMER_CC_CHANNEL0);
  uint32_t val = nrfx_timer_capture_get(&pcntTimer, NRF_TIMER_CC_CHANNEL0);
  return val;
}
