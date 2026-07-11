/*
 * Widget kit for the 128x64 mono panel: shell layout, list menu, dialog.
 */
#include <zephyr/sys/util.h>
#include <phone/ui.h>
#include "ui_priv.h"

#define STATUS_BAR_H 12
#define SOFTKEY_BAR_H 12

static lv_obj_t *content;

void ui_shell_create(lv_obj_t *screen)
{
	lv_obj_set_style_pad_all(screen, 0, 0);
	lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

	ui_statusbar_create(screen);

	content = lv_obj_create(screen);
	lv_obj_set_size(content, lv_pct(100),
			LV_VER_RES - STATUS_BAR_H - SOFTKEY_BAR_H);
	lv_obj_align(content, LV_ALIGN_TOP_MID, 0, STATUS_BAR_H);
	lv_obj_set_style_pad_all(content, 0, 0);
	lv_obj_set_style_border_width(content, 0, 0);
	lv_obj_set_style_radius(content, 0, 0);
	lv_obj_remove_flag(content, LV_OBJ_FLAG_SCROLLABLE);

	ui_softkeybar_create(screen);
}

lv_obj_t *ui_content_area(void)
{
	return content;
}

/* --- list --- */

lv_obj_t *ui_list_create(lv_obj_t *parent)
{
	lv_obj_t *list = lv_obj_create(parent);

	lv_obj_set_size(list, lv_pct(100), lv_pct(100));
	lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_all(list, 0, 0);
	lv_obj_set_style_pad_row(list, 1, 0);
	lv_obj_set_style_border_width(list, 0, 0);
	lv_obj_set_style_radius(list, 0, 0);
	lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_OFF);
	lv_obj_set_user_data(list, (void *)(intptr_t)-1);
	return list;
}

lv_obj_t *ui_list_add(lv_obj_t *list, const char *text)
{
	lv_obj_t *item = lv_label_create(list);

	lv_obj_set_width(item, lv_pct(100));
	lv_label_set_text(item, text);
	lv_label_set_long_mode(item, LV_LABEL_LONG_CLIP);
	lv_obj_set_style_pad_left(item, 2, 0);
	lv_obj_set_style_pad_top(item, 1, 0);
	lv_obj_set_style_pad_bottom(item, 1, 0);

	/* first item added becomes the selection */
	if (ui_list_selected(list) < 0) {
		ui_list_select(list, 0);
	}
	return item;
}

int ui_list_count(lv_obj_t *list)
{
	return (int)lv_obj_get_child_count(list);
}

int ui_list_selected(lv_obj_t *list)
{
	return (int)(intptr_t)lv_obj_get_user_data(list);
}

void ui_list_set_text(lv_obj_t *list, int index, const char *text)
{
	lv_obj_t *item = lv_obj_get_child(list, index);

	if (item != NULL) {
		lv_label_set_text(item, text);
	}
}

void ui_list_select(lv_obj_t *list, int index)
{
	int count = ui_list_count(list);

	if (count == 0) {
		return;
	}
	index = CLAMP(index, 0, count - 1);

	for (int i = 0; i < count; i++) {
		lv_obj_t *item = lv_obj_get_child(list, i);

		if (i == index) {
			lv_obj_set_style_bg_opa(item, LV_OPA_COVER, 0);
			lv_obj_set_style_bg_color(item, lv_color_black(), 0);
			lv_obj_set_style_text_color(item, lv_color_white(), 0);
		} else {
			lv_obj_set_style_bg_opa(item, LV_OPA_TRANSP, 0);
			lv_obj_set_style_text_color(item, lv_color_black(), 0);
		}
	}
	lv_obj_set_user_data(list, (void *)(intptr_t)index);
	lv_obj_scroll_to_view(lv_obj_get_child(list, index), LV_ANIM_OFF);
}

void ui_list_nav(lv_obj_t *list, int delta)
{
	ui_list_select(list, ui_list_selected(list) + delta);
}

/* --- dialog --- */

lv_obj_t *ui_dialog_open(const char *text)
{
	/* parent to the screen so it also covers the softkey bar's old labels */
	lv_obj_t *dlg = lv_obj_create(lv_screen_active());

	lv_obj_set_size(dlg, LV_HOR_RES - 8, LV_VER_RES - STATUS_BAR_H - 4);
	lv_obj_align(dlg, LV_ALIGN_BOTTOM_MID, 0, -2);
	lv_obj_set_style_radius(dlg, 0, 0);
	lv_obj_set_style_border_width(dlg, 1, 0);
	lv_obj_set_style_bg_color(dlg, lv_color_white(), 0);
	lv_obj_set_style_bg_opa(dlg, LV_OPA_COVER, 0);
	lv_obj_set_style_pad_all(dlg, 2, 0);
	lv_obj_remove_flag(dlg, LV_OBJ_FLAG_SCROLLABLE);

	lv_obj_t *label = lv_label_create(dlg);

	lv_label_set_text(label, text);
	lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
	lv_obj_set_width(label, lv_pct(100));
	lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);

	ui_softkeys_set(NULL, "OK");
	return dlg;
}

void ui_dialog_close(lv_obj_t *dialog)
{
	lv_obj_delete(dialog);
}
