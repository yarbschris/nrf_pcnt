#include <nrfx_gpiote.h>
#include <nrfx_timer.h>
#include <nrfx_ppi.h>

// pcntInit: Initialize Timer, GPIOTE, and PPI peripherals. Param pulsePin is input pin
nrfx_err_t pcntInit(int pulsePin);

// pcntClear: Clears the value of the counter
void pcntClear();

// pcntGetCount: Captures current value of counter and returns it
uint32_t pcntGetCount();