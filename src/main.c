#include <zephyr/kernel.h>
#include <nrfx_timer.h>

static const nrfx_timer_t timer0 = NRFX_TIMER_INSTANCE(0);

void setupTimer0() {
        nrfx_timer_config_t configTimer0 = NRFX_TIMER_DEFAULT_CONFIG(1000000);
        configTimer0.mode = NRF_TIMER_MODE_COUNTER;

        nrfx_err_t errCode = nrfx_timer_init(&timer0, &configTimer0, NULL);
        if (errCode != NRFX_SUCCESS) {
                printk("Init timer0 failed\n");
                printk("Error Code: %d\n", errCode);
        }
        else {
                printk("timer0 initialized\n");
        }
        nrfx_timer_enable(&timer0);
        printk("timer0 enabled\n");
}

int main(void)
{
        printk("Starting...\n");
        k_msleep(100);
        setupTimer0();
        printk("Finished Setup\n");
        return 0;
}
