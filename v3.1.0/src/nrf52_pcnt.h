#ifndef NRF52_PCNT
#define NRF52_PCNT

#include <nrfx_gpiote.h>
#include <nrfx_ppi.h>
#include <nrfx_timer.h>

// pcntInit: Initialize pulse counter on param pin. The count is latched by
// hardware once, measureTimeMs after init or the last pcntClearTimer()
nrfx_err_t pcntInit(int pin, uint16_t measureTimeMs);

// pcntClearCounter: Resets value of counter to zero
void pcntClearCounter(void);

// pcntClearTimer: Resets value of timer to zero (restarts measurement window)
void pcntClearTimer(void);

// pcntCaptureAndGetCount: Capture current value of counter and return it
// Since we're using PPI to capture the counter value, should not need to used
// this function, and pcntGetCount should be used instead
uint32_t pcntCaptureAndGetCount(void);

// pcntGetCount: Get the last captured value of counter
uint32_t pcntGetCount(void);

#endif // NRF52_PCNT
