#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
#include <modem/modem_info.h>
#include <modem/nrf_modem_lib.h>

LOG_MODULE_REGISTER(main);

int main(void)
{
    int err;
    err = nrf_modem_lib_init();
    if(err < 0)
        LOG_ERR("Unable to initialize modem lib. (err: %d)", err);

    LOG_INF("=====Battery Voltage Sample=====");
    
    modem_info_init();
    
    /* Get battery voltage */
    int voltage = 0;
    err = modem_info_get_batt_voltage(&voltage);
    if(err < 0) {
        LOG_ERR("Failed to read battery voltage %d" err);
        return 0;
    }
    LOG_INF("Battery Voltage: %d mV", voltage);

    return 0;
}