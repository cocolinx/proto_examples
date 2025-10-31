#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <nrf_modem.h> 
#include <nrf_modem_at.h> 
#include <modem/nrf_modem_lib.h>
#include <zephyr/drivers/flash.h>

#define SPI_FLASH_TEST_REGION_OFFSET 0xff000
#define SPI_FLASH_SECTOR_SIZE        4096 /* 4kB */

LOG_MODULE_REGISTER(main_flash, CONFIG_LOG_DEFAULT_LEVEL);

static const struct device *flash_dev = DEVICE_DT_GET(DT_ALIAS(by25q0));

const uint8_t erased[] = { 0xff, 0xff, 0xff, 0xff , 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

void single_sector_test(const struct device *flash_dev)
{
	const uint8_t expected[] = "hello world";

	const size_t len = sizeof(expected);
	uint8_t buf[sizeof(expected)];
	int rc;

	LOG_INF("Perform test on single sector");
	/* Write protection needs to be disabled before each write or
	 * erase, since the flash component turns on write protection
	 * automatically after completion of write and erase
	 * operations.
	 */
	LOG_INF("Test 1: Flash erase");

	/* Full flash erase if SPI_FLASH_TEST_REGION_OFFSET = 0 and
	 * SPI_FLASH_SECTOR_SIZE = flash size
	 */
	rc = flash_erase(flash_dev, SPI_FLASH_TEST_REGION_OFFSET,
			 SPI_FLASH_SECTOR_SIZE);
	if (rc != 0) {
		LOG_INF("Flash erase failed! %d", rc);
	} else {
		/* Check erased pattern */
		memset(buf, 0, len);
		rc = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, buf, len);
		if (rc != 0) {
			LOG_INF("Flash read failed! %d", rc);
			return;
		}
		if (memcmp(erased, buf, len) != 0) {
			LOG_INF("Flash erase failed at offset 0x%x got 0x%x",
				SPI_FLASH_TEST_REGION_OFFSET, *(uint32_t *)buf);
			return;
		}
		LOG_INF("Flash erase succeeded!");
	}
	LOG_INF("Test 2: Flash write");

	LOG_INF("Attempting to write %zu bytes", len);
	rc = flash_write(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, expected, len);
	if (rc != 0) {
		LOG_INF("Flash write failed! %d", rc);
		return;
	}

	memset(buf, 0, len);
	rc = flash_read(flash_dev, SPI_FLASH_TEST_REGION_OFFSET, buf, len);
	if (rc != 0) {
		LOG_INF("Flash read failed! %d", rc);
		return;
	}

	if (memcmp(expected, buf, len) == 0) {
		LOG_INF("Data read matches data written. Good!!");
		LOG_INF("read data: %.*s", len, buf);
	} else {
		const uint8_t *wp = expected;
		const uint8_t *rp = buf;
		const uint8_t *rpe = rp + len;

		LOG_INF("Data read does not match data written!!");
		while (rp < rpe) {
			LOG_INF("%08x wrote %02x read %02x %s",
			       (uint32_t)(SPI_FLASH_TEST_REGION_OFFSET + (rp - buf)),
			       *wp, *rp, (*rp == *wp) ? "match" : "MISMATCH");
			++rp;
			++wp;
		}
	}
}

int main(void)
{
    nrf_modem_lib_init();
    LOG_INF("=====FLASH EXAMPLE=====");
    
    if (!device_is_ready(flash_dev)) {
		LOG_INF("%s: device not ready.", flash_dev->name);
		return 0;
	}

	single_sector_test(flash_dev);

    return 0;
}
