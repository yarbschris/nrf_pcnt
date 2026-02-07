#ifndef NRF52_PCNT
#define NRF52_PCNT

#include <nrfx.h>
#include <nrfx_gpiotee.h>
#include <nrfx_ppi.h>
#include <nrfx_timer.h>

// pcntInit: Initialize pulse counter on param pin
nrfx_err_t pcntInit(int pin);

// pcntClear: Resets value of counter to zero
void pcntClear(void);

// pcntGetCount: Capture current value of counter and return it
uint32_t pcntGetCount(void);
