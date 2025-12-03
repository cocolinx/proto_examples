#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

/* size of stack area used by each thread */
#define STACKSIZE 1024
/* scheduling priority used by each thread */
#define PRIORITY 7

LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

static void blink(uint32_t sleep_ms)
{
    if(!device_is_ready(led0.port)) return;

    int err;
    err = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
    if(err < 0) {
        LOG_ERR("Failed to config gpio pin %d", err);
        return;
    }
    while(true) {
        err = gpio_pin_set_dt(&led0, 1); /* turn on */
        if(err < 0) {
            LOG_ERR("Failed to set gpio pin %d", err);
            break;
        }
        LOG_INF("turn on");
        k_msleep(sleep_ms);
        err = gpio_pin_set_dt(&led0, 0); /* turn off */
        if(err < 0) {
            LOG_ERR("Failed to set gpio pin %d", err);
            break;
        }
        LOG_INF("turn off");
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
