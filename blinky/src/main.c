#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <nrf_modem.h> 
#include <nrf_modem_at.h> 
#include <modem/nrf_modem_lib.h>
#include <zephyr/drivers/gpio.h>

#define PIN_LED_0       16
#define SLEEP_TIME_MS   1000

LOG_MODULE_REGISTER(main_blinky, CONFIG_LOG_DEFAULT_LEVEL);

static const struct device *leds = DEVICE_DT_GET(DT_NODELABEL(gpio0));

int main(void)
{
    nrf_modem_lib_init();
    
    LOG_INF("=====BLINKY EXAMPLE=====");
    
    int8_t ret;
    int iter = 5;

    if(!device_is_ready(leds)) return 0;

    // low(on), high(off)
    gpio_pin_configure(leds, PIN_LED_0, GPIO_OUTPUT_HIGH);
    
    while(iter--){
        ret = gpio_pin_set_raw(leds, PIN_LED_0, 0);
        LOG_INF("turn on: %d", PIN_LED_0);
        k_msleep(SLEEP_TIME_MS);
        ret = gpio_pin_set_raw(leds, PIN_LED_0, 1);
        LOG_INF("turn off: %d", PIN_LED_0);
        k_msleep(SLEEP_TIME_MS);
    }

    LOG_INF("main close...");
    return 0;
}
