/*
 * Settings: contrast (live, Left/Right) + About dialog.
 * Persistence comes with the settings subsystem on LittleFS (post-M1).
 */
#include <stdio.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/version.h>
#include <phone/app.h>
#include <phone/ui.h>

#define ITEM_CONTRAST 0
#define ITEM_ABOUT 1

static const struct device *const display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

static lv_obj_t *menu;
static lv_obj_t *dialog;
static uint8_t contrast = PHONE_DISPLAY_DEFAULT_CONTRAST;

static void contrast_label_update(void)
{
	char buf[24];

	snprintf(buf, sizeof(buf), "Contrast  < %u >", contrast);
	ui_list_set_text(menu, ITEM_CONTRAST, buf);
}

static void settings_resume(struct app_ctx *ctx)
{
	menu = ui_list_create(ctx->root);
	dialog = NULL;

	ui_list_add(menu, "");
	ui_list_add(menu, "About");
	contrast_label_update();

	ui_softkeys_set("Select", "Back");
}

static void contrast_adjust(int delta)
{
	contrast = CLAMP((int)contrast + delta, 0, 63);
	display_set_contrast(display, contrast);
	contrast_label_update();
}

static bool settings_on_event(struct app_ctx *ctx, const struct phone_event *evt)
{
	ARG_UNUSED(ctx);

	if (evt->type != PHONE_EVENT_KEY) {
		return false;
	}
	if (evt->key.action != KEY_ACTION_PRESS &&
	    evt->key.action != KEY_ACTION_REPEAT) {
		return true;
	}

	if (dialog != NULL) {
		if (evt->key.key == KEY_SOFT_R || evt->key.key == KEY_SELECT ||
		    evt->key.key == KEY_END) {
			ui_dialog_close(dialog);
			dialog = NULL;
			ui_softkeys_set("Select", "Back");
		}
		return true;
	}

	switch (evt->key.key) {
	case KEY_UP:
		ui_list_nav(menu, -1);
		return true;
	case KEY_DOWN:
		ui_list_nav(menu, +1);
		return true;
	case KEY_LEFT:
	case KEY_RIGHT:
		if (ui_list_selected(menu) == ITEM_CONTRAST) {
			contrast_adjust(evt->key.key == KEY_RIGHT ? +5 : -5);
		}
		return true;
	case KEY_SELECT:
	case KEY_SOFT_L:
		if (ui_list_selected(menu) == ITEM_ABOUT) {
			dialog = ui_dialog_open("esp32PhoneV2\nZephyr " KERNEL_VERSION_STRING);
		}
		return true;
	default:
		return false;	/* soft_r/end fall through: framework pops */
	}
}

PHONE_APP_DEFINE(settings_app,
	.name = "Settings",
	.resume = settings_resume,
	.on_event = settings_on_event,
);
