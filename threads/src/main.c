#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

/* size of stack area used by each thread */
#define STACKSIZE 1024
/* scheduling priority used by each thread */
#define PRIORITY 7
#define PIN_LED_0 16

LOG_MODULE_REGISTER(main_blinky, CONFIG_LOG_DEFAULT_LEVEL);

static const struct device *led = DEVICE_DT_GET(DT_NODELABEL(gpio0));

static void blink(uint32_t sleep_ms)
{
    if(!device_is_ready(led)) return;

    int err;
    err = gpio_pin_configure(led, PIN_LED_0, GPIO_OUTPUT_HIGH);
    if(err < 0) {
        LOG_ERR("Failed to config gpio %d pin %d", PIN_LED_0, err);
        return;
    }
    while(true) {
        err = gpio_pin_set_raw(led, PIN_LED_0, 0); /* turn on */
        if(err < 0) {
            LOG_ERR("Failed to set gpio %d pin %d", PIN_LED_0, err);
            break;
        }
        LOG_INF("turn on (PIN: %d)", PIN_LED_0);
        k_msleep(sleep_ms);
        err = gpio_pin_set_raw(led, PIN_LED_0, 1); /* turn off */
        if(err < 0) {
            LOG_ERR("Failed to set gpio %d pin %d", PIN_LED_0, err);
            break;
        }
        LOG_INF("turn off (PIN: %d)", PIN_LED_0);
        k_msleep(sleep_ms);
    }
}

static void log_print(uint32_t sleep_ms)
{
    while(true) {
        LOG_INF("This is from log_print");
        k_msleep(sleep_ms);
    }
}

static void blink0(void)
{
    blink(1000);
}

static void log_print0(void)
{
    log_print(2000);
}

K_THREAD_DEFINE(blink0_id, STACKSIZE, blink0, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(log_print0_id, STACKSIZE, log_print0, NULL, NULL, NULL, PRIORITY, 0, 0);
