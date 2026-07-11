/*
 * The app contract. Built-in apps register at link time via PHONE_APP_DEFINE;
 * SD-loaded apps (M4) implement the same struct, so this header is the ABI.
 */
#ifndef PHONE_APP_H_
#define PHONE_APP_H_

#include <stdbool.h>
#include <stddef.h>
#include <zephyr/sys/iterable_sections.h>
#include <lvgl.h>
#include <phone/event.h>

#define PHONE_APP_ABI_VERSION 1

struct app_ctx {
	lv_obj_t *root;	/* app's LVGL parent; created on resume, deleted on pause */
	void *state;	/* app-private, survives pause/resume */
};

struct phone_app {
	const char *name;	/* registry key and menu label */
	uint32_t abi_version;
	int  (*create)(struct app_ctx *ctx);	/* instance allocated (optional) */
	void (*resume)(struct app_ctx *ctx);	/* foreground: build UI in ctx->root */
	void (*pause)(struct app_ctx *ctx);	/* lost foreground: root is deleted after this */
	void (*destroy)(struct app_ctx *ctx);	/* popped off the stack (optional) */
	/* return true if consumed; false lets the framework handle it (Back/Home) */
	bool (*on_event)(struct app_ctx *ctx, const struct phone_event *evt);
};

#define PHONE_APP_DEFINE(var, ...)					\
	static const STRUCT_SECTION_ITERABLE(phone_app, var) = {	\
		.abi_version = PHONE_APP_ABI_VERSION,			\
		__VA_ARGS__						\
	}

/* Registry (built-ins now; storage service adds SD apps in M4) */
size_t phone_app_count(void);
const struct phone_app *phone_app_by_index(size_t idx);
const struct phone_app *phone_app_find(const char *name);

/* Screen stack — only callable from the UI thread (i.e. from app callbacks) */
int  phone_app_launch(const char *name);	/* push on top of current */
void phone_app_back(void);			/* pop one */
void phone_app_home(void);			/* pop to the root app */

#endif /* PHONE_APP_H_ */
