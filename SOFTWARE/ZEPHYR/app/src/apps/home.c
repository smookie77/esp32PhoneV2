/*
 * Home: root menu listing every registered app. Never popped.
 */
#include <string.h>
#include <phone/app.h>
#include <phone/ui.h>

#define MENU_MAX 16

static lv_obj_t *menu;
static const struct phone_app *entries[MENU_MAX];
static int entry_count;
static int last_selected;	/* restore selection when returning home */

static void home_resume(struct app_ctx *ctx)
{
	menu = ui_list_create(ctx->root);
	entry_count = 0;

	for (size_t i = 0; i < phone_app_count() && entry_count < MENU_MAX; i++) {
		const struct phone_app *app = phone_app_by_index(i);

		if (strcmp(app->name, "Home") == 0) {
			continue;
		}
		entries[entry_count++] = app;
		ui_list_add(menu, app->name);
	}

	ui_list_select(menu, last_selected);
	ui_softkeys_set("Open", NULL);
}

static bool home_on_event(struct app_ctx *ctx, const struct phone_event *evt)
{
	ARG_UNUSED(ctx);

	if (evt->type != PHONE_EVENT_KEY) {
		return false;
	}
	if (evt->key.action != KEY_ACTION_PRESS &&
	    evt->key.action != KEY_ACTION_REPEAT) {
		return true;	/* root screen swallows everything else */
	}

	switch (evt->key.key) {
	case KEY_UP:
		ui_list_nav(menu, -1);
		break;
	case KEY_DOWN:
		ui_list_nav(menu, +1);
		break;
	case KEY_SELECT:
	case KEY_SOFT_L:
		last_selected = ui_list_selected(menu);
		if (entry_count > 0) {
			phone_app_launch(entries[last_selected]->name);
		}
		break;
	default:
		break;
	}
	return true;
}

PHONE_APP_DEFINE(home_app,
	.name = "Home",
	.resume = home_resume,
	.on_event = home_on_event,
);
