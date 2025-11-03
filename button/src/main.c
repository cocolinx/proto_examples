#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <nrf_modem.h> 
#include <nrf_modem_at.h> 
#include <modem/nrf_modem_lib.h>
#include <zephyr/drivers/gpio.h>

/* onboard button pin num */
#define PIN_BUTTON_0 23
/* prevent chattering */
#define DEBOUNCE_MS 20

LOG_MODULE_REGISTER(main_button, CONFIG_LOG_DEFAULT_LEVEL);
static const struct device *btn = DEVICE_DT_GET(DT_NODELABEL(gpio0));

static void btn_debounce_work_handler(struct k_work *work);
K_WORK_DELAYABLE_DEFINE(btn_debounce_work, btn_debounce_work_handler);
static struct gpio_callback btn_cb;

static void btn_isr(const struct device *port, struct gpio_callback *cb, uint32_t pins)
{
    (void)k_work_reschedule(&btn_debounce_work, K_MSEC(DEBOUNCE_MS));
}

static void btn_debounce_work_handler(struct k_work *work)
{
    int ret = gpio_pin_get_raw(btn, PIN_BUTTON_0); 
    if(ret < 0) {
        LOG_ERR("Failed to get gpio pin status %d", ret);
        return;
    }
    bool pressed = (ret == 0) ? true : false;
    if(pressed) LOG_INF("button pressed");
    else LOG_INF("button released");
}

int main(void)
{
    int err;
    err = nrf_modem_lib_init();
    if(err < 0)
        LOG_ERR("Unable to initialize modem lib. (err: %d)", err);

    LOG_INF("=====BUTTON EXAMPLE=====");
    
    err = gpio_pin_configure(btn, PIN_BUTTON_0, GPIO_INPUT);
    if(err < 0) {
        LOG_ERR("Failed to config gpio pin %d", err);
        return 0;
    }
    err = gpio_pin_interrupt_configure(btn, PIN_BUTTON_0, GPIO_INT_DISABLE);
    if(err < 0) {
        LOG_ERR("Failed to config gpio pin interrupt %d", err);
        return 0;
    }

    gpio_init_callback(&btn_cb, btn_isr, BIT(PIN_BUTTON_0));
    err = gpio_add_callback(btn, &btn_cb);
    if(err < 0) {
        LOG_ERR("Failed to add gpio callback %d", err);
        return 0;
    }
    err = gpio_pin_interrupt_configure(btn, PIN_BUTTON_0, GPIO_INT_EDGE_BOTH); /* rising | falling */
    if(err < 0) {
        LOG_ERR("Failed to config gpio pin interrupt %d", err);
        return 0;
    }

    LOG_INF("Press button to test!");

    return 0;
}
