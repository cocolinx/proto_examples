#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <nrf_modem.h> 
#include <nrf_modem_at.h> 
#include <modem/nrf_modem_lib.h>
#include <zephyr/drivers/gpio.h>

/* onboard LED pin num */
#define PIN_LED_0       16
/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

LOG_MODULE_REGISTER(main_blinky, CONFIG_LOG_DEFAULT_LEVEL);

static const struct device *led = DEVICE_DT_GET(DT_NODELABEL(gpio0));

int main(void)
{
    int err;
    err = nrf_modem_lib_init();
    if(err < 0)
        LOG_ERR("Unable to initialize modem lib. (err: %d)", err);

    LOG_INF("=====BLINKY EXAMPLE=====");

    if(!device_is_ready(led)) return 0;

    /* low(on), high(off) */
    err = gpio_pin_configure(led, PIN_LED_0, GPIO_OUTPUT_HIGH);
    if(err < 0) {
        LOG_ERR("Failed to config gpio pin %d", err);
        return 0;
    }
    while(true){
        /* turn on */
        err = gpio_pin_set_raw(led, PIN_LED_0, 0);
        if(err < 0) {
            LOG_ERR("Failed to set gpio pin %d", err);
            break;
        }
        LOG_INF("turn on (PIN: %d)", PIN_LED_0);
        k_msleep(SLEEP_TIME_MS);

        /* turn off */
        err = gpio_pin_set_raw(led, PIN_LED_0, 1);
        if(err < 0) {
            LOG_ERR("Failed to set gpio pin %d", err);
            break;
        }
        LOG_INF("turn off (PIN: %d)", PIN_LED_0);
        k_msleep(SLEEP_TIME_MS);
    }

    return 0;
}
