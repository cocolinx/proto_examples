#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <modem/nrf_modem_lib.h>
#include <stdio.h>
#include <zephyr/drivers/i2c.h>

LOG_MODULE_REGISTER(main_accel, CONFIG_LOG_DEFAULT_LEVEL);

K_SEM_DEFINE(sem, 0, 1); /* starts off "not available" */

static const struct device *const dev = DEVICE_DT_GET(DT_ALIAS(accel0));


static void trigger_handler(const struct device *dev, const struct sensor_trigger *trigger)
{
	ARG_UNUSED(trigger);

	/* Always fetch the sample to clear the data ready interrupt in the
	 * sensor.
	 */
    int err;
    err = sensor_sample_fetch_chan(dev, SENSOR_CHAN_ACCEL_XYZ);
    if (err) {
        LOG_ERR("sensor_sample_fetch failed: %s, %d", strerror(-err), err);
        return;
    }
	k_sem_give(&sem);
}

int main(void)
{
    int err;
    err = nrf_modem_lib_init();
    if(err < 0) {
        LOG_ERR("Unable to initialize modem lib. (err: %d)", err);
        return 0;
    }

    LOG_INF("=====ACCELEROMETER EXAMPLE=====");

    struct sensor_value data[3];
    struct sensor_trigger trig = {
            .type = SENSOR_TRIG_DATA_READY,
            .chan = SENSOR_CHAN_ACCEL_XYZ,
    };

    if (!device_is_ready(dev)) {
        LOG_ERR("Device %s is not ready\n", dev->name);
        return 0;
    }

    err = sensor_trigger_set(dev, &trig, trigger_handler);
    if (err != 0) {
        LOG_ERR("Could not set trigger, error %d", err);
        return 0;
    }


    while (1) {
        k_sem_take(&sem, K_FOREVER);

        sensor_channel_get(dev, SENSOR_CHAN_ACCEL_XYZ, data);
        /* Print accel x,y,z data */
        LOG_INF("%s [m/s^2]: (%8.6f, %8.6f, %8.6f)\n", dev->name,
                sensor_value_to_double(&data[0]), sensor_value_to_double(&data[1]), sensor_value_to_double(&data[2]));
    }
}
