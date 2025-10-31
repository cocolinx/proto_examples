#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <modem/nrf_modem_lib.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/adc.h>
#include <hal/nrf_saadc.h>

#define AIN_TMP23X      NRF_SAADC_INPUT_AIN0
#define PIN_TMP23X_VDD  28 

LOG_MODULE_REGISTER(main_temperature, CONFIG_LOG_DEFAULT_LEVEL);

static const struct device *tmp_vdd = DEVICE_DT_GET(DT_NODELABEL(gpio0));
static const struct device *tmp_dev = DEVICE_DT_GET(DT_NODELABEL(adc));
static struct adc_channel_cfg channel_cfg;
static struct adc_sequence adc_sequence;
static uint16_t adc_buffer;

int main(void)
{
    nrf_modem_lib_init();
    LOG_INF("======TEMPERATURE EXAMPLE=====");

    int mv;
    float celsius;

    gpio_pin_configure(tmp_vdd, PIN_TMP23X_VDD, GPIO_OUTPUT_HIGH);

    /* adc init */
    channel_cfg.channel_id = 1;
    channel_cfg.gain = ADC_GAIN_1_5; /* 3.0v */
    channel_cfg.reference = ADC_REF_INTERNAL;
    channel_cfg.acquisition_time = ADC_ACQ_TIME(ADC_ACQ_TIME_MICROSECONDS, 40);
    channel_cfg.differential = false;
    channel_cfg.input_positive = AIN_TMP23X; 
    channel_cfg.input_negative = NRF_SAADC_INPUT_DISABLED;

    adc_sequence.options = NULL;
    adc_sequence.channels = BIT(1);
    adc_sequence.buffer = &adc_buffer;
    adc_sequence.buffer_size = 2;
    adc_sequence.resolution = 14;
    adc_sequence.oversampling = 4;
    adc_sequence.calibrate = true;

    adc_channel_setup(tmp_dev, &channel_cfg);

    while(true) {
        adc_read(tmp_dev, &adc_sequence);
        mv = adc_buffer;
        mv = (mv * 3000) / 16383; /* mv: 0~3000 */
        celsius = (float)((mv - 500)) / 10.0f;
        LOG_INF("temperature(celsius): %f", celsius);
        k_msleep(1000);
    }

    return 0;
}
