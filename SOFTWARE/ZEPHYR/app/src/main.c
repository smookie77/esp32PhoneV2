/*
 * esp32PhoneV2 — Zephyr firmware
 * Boot: display up → framework (UI shell + home app) → event loop.
 * The main thread becomes the UI thread; services run on their own threads.
 */

#include <zephyr/kernel.h>
#include <zephyr/version.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>
#include <phone/ui.h>
#include "core/framework.h"

LOG_MODULE_REGISTER(phone, LOG_LEVEL_INF);

int main(void)
{
	LOG_INF("esp32PhoneV2 booting (Zephyr " KERNEL_VERSION_STRING ")");

	const struct device *display = DEVICE_DT_GET_OR_NULL(DT_CHOSEN(zephyr_display));

	if (display == NULL || !device_is_ready(display)) {
		LOG_WRN("display not ready — running headless (shell still available)");
		return 0;
	}

	display_set_contrast(display, PHONE_DISPLAY_DEFAULT_CONTRAST);

	framework_init();

	display_blanking_off(display);

	framework_run();	/* never returns */
	return 0;
}
