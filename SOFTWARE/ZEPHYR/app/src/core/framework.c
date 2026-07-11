/*
 * App framework: screen stack, event routing, LVGL main loop.
 * Everything here runs on the UI thread (main), the only LVGL context.
 */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <phone/app.h>
#include <phone/channels.h>
#include <phone/ui.h>
#include "../ui/ui_priv.h"
#include "framework.h"

LOG_MODULE_REGISTER(framework, LOG_LEVEL_INF);

#define STACK_MAX 8

struct screen {
	const struct phone_app *app;
	struct app_ctx ctx;
};

static struct screen stack[STACK_MAX];
static int depth;

/* Input events cross from the input service thread to here via a msgq. */
K_MSGQ_DEFINE(input_q, sizeof(struct phone_key_event), 16, 4);

static void input_listener_cb(const struct zbus_channel *chan)
{
	const struct phone_key_event *evt = zbus_chan_const_msg(chan);

	if (k_msgq_put(&input_q, evt, K_NO_WAIT) != 0) {
		LOG_WRN("input queue full, dropping %s", phone_key_name(evt->key));
	}
}

ZBUS_LISTENER_DEFINE(framework_input_listener, input_listener_cb);
ZBUS_CHAN_ADD_OBS(chan_input, framework_input_listener, 3);

static struct screen *top(void)
{
	return depth > 0 ? &stack[depth - 1] : NULL;
}

/* Give the app a fresh root container and foreground status. */
static void activate(struct screen *scr)
{
	scr->ctx.root = lv_obj_create(ui_content_area());
	lv_obj_set_size(scr->ctx.root, lv_pct(100), lv_pct(100));
	lv_obj_set_style_pad_all(scr->ctx.root, 0, 0);
	lv_obj_set_style_border_width(scr->ctx.root, 0, 0);
	lv_obj_set_style_radius(scr->ctx.root, 0, 0);

	ui_statusbar_set_title(scr->app->name);
	ui_softkeys_set(NULL, NULL);	/* apps set their own in resume() */

	if (scr->app->resume != NULL) {
		scr->app->resume(&scr->ctx);
	}
}

static void deactivate(struct screen *scr)
{
	if (scr->app->pause != NULL) {
		scr->app->pause(&scr->ctx);
	}
	lv_obj_delete(scr->ctx.root);
	scr->ctx.root = NULL;
}

int phone_app_launch(const char *name)
{
	const struct phone_app *app = phone_app_find(name);

	if (app == NULL) {
		LOG_ERR("no such app: %s", name);
		return -ENOENT;
	}
	if (depth >= STACK_MAX) {
		LOG_ERR("screen stack full, can't launch %s", name);
		return -ENOMEM;
	}
	if (app->abi_version != PHONE_APP_ABI_VERSION) {
		LOG_ERR("%s: ABI %u != %u", name, app->abi_version, PHONE_APP_ABI_VERSION);
		return -EINVAL;
	}

	if (top() != NULL) {
		deactivate(top());
	}

	struct screen *scr = &stack[depth];

	scr->app = app;
	scr->ctx = (struct app_ctx){ 0 };
	if (app->create != NULL) {
		int err = app->create(&scr->ctx);

		if (err != 0) {
			LOG_ERR("%s: create failed (%d)", name, err);
			if (top() != NULL) {
				activate(top());
			}
			return err;
		}
	}
	depth++;

	LOG_INF("launch %s (depth %d)", name, depth);
	activate(scr);
	return 0;
}

void phone_app_back(void)
{
	if (depth <= 1) {
		return;	/* never pop the root app */
	}

	struct screen *scr = top();

	deactivate(scr);
	if (scr->app->destroy != NULL) {
		scr->app->destroy(&scr->ctx);
	}
	depth--;

	LOG_INF("back to %s (depth %d)", top()->app->name, depth);
	activate(top());
}

void phone_app_home(void)
{
	while (depth > 1) {
		struct screen *scr = top();

		deactivate(scr);
		if (scr->app->destroy != NULL) {
			scr->app->destroy(&scr->ctx);
		}
		depth--;
	}
	activate(top());
}

static void dispatch(const struct phone_key_event *key)
{
	struct screen *scr = top();

	if (scr == NULL) {
		return;
	}

	const struct phone_event evt = {
		.type = PHONE_EVENT_KEY,
		.key = *key,
	};

	bool consumed = false;

	if (scr->app->on_event != NULL) {
		consumed = scr->app->on_event(&scr->ctx, &evt);
	}

	if (!consumed && key->action == KEY_ACTION_PRESS) {
		/* Framework defaults: right softkey = back, End = home. */
		if (key->key == KEY_SOFT_R) {
			phone_app_back();
		} else if (key->key == KEY_END) {
			phone_app_home();
		}
	}
}

void framework_init(void)
{
#ifdef CONFIG_LV_USE_THEME_MONO
	lv_display_t *disp = lv_display_get_default();

	lv_display_set_theme(disp, lv_theme_mono_init(disp, false, LV_FONT_DEFAULT));
#endif

	ui_shell_create(lv_screen_active());

	if (phone_app_launch("Home") != 0) {
		LOG_ERR("home app missing — check registry");
	}
}

void framework_run(void)
{
	struct phone_key_event key;

	for (;;) {
		uint32_t next = lv_timer_handler();

		if (k_msgq_get(&input_q, &key, K_MSEC(MIN(next, 50))) == 0) {
			dispatch(&key);
			/* drain whatever queued while rendering */
			while (k_msgq_get(&input_q, &key, K_NO_WAIT) == 0) {
				dispatch(&key);
			}
		}
	}
}
