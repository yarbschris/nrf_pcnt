# nrf_pcnt

Hardware pulse counter for the nRF52 series. Counts edges on a GPIO pin and captures the total over a fixed time window without waking the CPU.

The nRF52 has no dedicated pulse-counter peripheral, unlike the ESP32's PCNT. This library builds one from GPIOTE, PPI, and two TIMER instances, so counting continues while the CPU sleeps. It was written for the [Moire](https://github.com/moire-cs/firmware) soil-moisture sensor nodes deployed in Ecuador, where the moisture probe outputs a frequency and the node spends most of its life asleep.

## How it works

```
Pulse pin ──► GPIOTE IN event ──PPI──► TIMER2 COUNT task      (counter, 32-bit)
TIMER3 COMPARE[1] event ──────PPI──► TIMER2 CAPTURE[0] task   (gate, 1 MHz)
```

1. **GPIOTE** generates an event on each high-to-low transition of the pulse pin (internal pull-up enabled).
2. **PPI channel 1** connects that event to the `COUNT` task of TIMER2, which runs in counter mode. Each pulse increments it in hardware.
3. **TIMER3** runs in timer mode at 1 MHz with a compare value set to the measurement window.
4. **PPI channel 2** connects TIMER3's compare event to TIMER2's `CAPTURE0` task. When the window elapses, the pulse count is latched into a capture register.

After setup, the CPU is not involved.

## API

```c
#include "nrf52_pcnt.h"

// Set up the counter on `pin` with a gated window of `measureTimeMs`.
nrfx_err_t pcntInit(int pin, uint16_t measureTimeMs);

// Reset the pulse count / restart the measurement window.
void pcntClearCounter(void);
void pcntClearTimer(void);

// Count latched at the end of the last window.
uint32_t pcntGetCount(void);

// Capture and return the live count right now (bypasses the window).
uint32_t pcntCaptureAndGetCount(void);
```

## Usage

```c
#define PULSE_PIN       NRF_GPIO_PIN_MAP(0, 3)
#define MEASURE_TIME_MS 1000

if (pcntInit(PULSE_PIN, MEASURE_TIME_MS) != NRFX_SUCCESS) {
    // handle error
}

// Start a measurement
pcntClearCounter();
pcntClearTimer();

// ... sleep for at least MEASURE_TIME_MS ...

uint32_t pulses = pcntGetCount();
float frequency_hz = pulses * 1000.0f / MEASURE_TIME_MS;
```

## Resources used

| Peripheral | Instance   | Purpose                                        |
| ---------- | ---------- | ---------------------------------------------- |
| TIMER      | 2          | Pulse counter (counter mode, 32-bit)           |
| TIMER      | 3          | Measurement window (timer mode, 1 MHz, 32-bit) |
| PPI        | 2 channels | Pin → count, window end → capture              |
| GPIOTE     | 1 channel  | Edge detection on the pulse pin                |

TIMER0 is avoided because the SoftDevice and many RTOS ports reserve it. Change the instances at the top of `nrf52_pcnt.c` if they conflict with your application.

## Versions

| Directory | nrfx version | Notes                                                      |
| --------- | ------------ | ---------------------------------------------------------- |
| `v2.1.0/` | nrfx 2.x     | Arduino/PlatformIO (Adafruit nRF52 core), as used in Moire |
| `v3.1.0/` | nrfx 3.x     | nRF Connect SDK / Zephyr                                   |

## Building

**nRF Connect SDK / Zephyr (v3.1.0):** the repo includes `CMakeLists.txt`, `prj.conf`, and a devicetree overlay for the nRF52840 DK.

```sh
west build -b nrf52840dk/nrf52840
west flash
```

**PlatformIO / Arduino (v2.1.0):** copy `nrf52_pcnt.c` and `nrf52_pcnt.h` into your project's `src/`.
