#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <nrf_modem.h> 
#include <nrf_modem_at.h> 
#include <modem/nrf_modem_lib.h>
#include <zephyr/drivers/gpio.h>

LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

int main(void)
{
    int err;
    err = nrf_modem_lib_init();
    if(err < 0)
        LOG_ERR("Unable to initialize modem lib. (err: %d)", err);

    LOG_INF("=====BLINKY EXAMPLE=====");

    if(!device_is_ready(led0.port)) 
    {
        LOG_ERR("LED device not ready");
        return 0;
    }

    /* low(on), high(off) */
    err = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
    if(err < 0) 
    {
        LOG_ERR("Failed to config LED %d", err);
        return 0;
    }

    while(true)
    {
        /* turn on */
        err = gpio_pin_set_dt(&led0, 1);
        if(err < 0) {
            LOG_ERR("Failed to set gpio pin %d", err);
            break;
        }
        k_msleep(1000);

        /* turn off */
        err = gpio_pin_set_dt(&led0, 0);
        if(err < 0) {
            LOG_ERR("Failed to set gpio pin %d", err);
            break;
        }
        k_msleep(1000);
    }

    return 0;
}
