/*
 * Status bar: 12 px strip at the top. Title now; clock/battery/signal
 * get real data when the time/power/modem services land (M2/M3).
 */
#include <phone/ui.h>
#include "ui_priv.h"

#define BAR_H 12

static lv_obj_t *title_label;
static lv_obj_t *right_label;

void ui_statusbar_create(lv_obj_t *parent)
{
	lv_obj_t *bar = lv_obj_create(parent);

	lv_obj_set_size(bar, lv_pct(100), BAR_H);
	lv_obj_align(bar, LV_ALIGN_TOP_MID, 0, 0);
	lv_obj_set_style_pad_all(bar, 0, 0);
	lv_obj_set_style_pad_left(bar, 1, 0);
	lv_obj_set_style_pad_right(bar, 1, 0);
	lv_obj_set_style_radius(bar, 0, 0);
	lv_obj_set_style_border_width(bar, 1, 0);
	lv_obj_set_style_border_side(bar, LV_BORDER_SIDE_BOTTOM, 0);
	lv_obj_remove_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

	title_label = lv_label_create(bar);
	lv_label_set_text(title_label, "");
	lv_label_set_long_mode(title_label, LV_LABEL_LONG_CLIP);
	lv_obj_align(title_label, LV_ALIGN_LEFT_MID, 0, 0);

	right_label = lv_label_create(bar);
	lv_label_set_text(right_label, "--%");
	lv_obj_align(right_label, LV_ALIGN_RIGHT_MID, 0, 0);
}

void ui_statusbar_set_title(const char *title)
{
	lv_label_set_text(title_label, title != NULL ? title : "");
}
