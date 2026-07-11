/*
 * Softkey bar: 12 px strip at the bottom, labels for the two softkeys.
 */
#include <phone/ui.h>
#include "ui_priv.h"

#define BAR_H 12

static lv_obj_t *left_label;
static lv_obj_t *right_label;

void ui_softkeybar_create(lv_obj_t *parent)
{
	lv_obj_t *bar = lv_obj_create(parent);

	lv_obj_set_size(bar, lv_pct(100), BAR_H);
	lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, 0);
	lv_obj_set_style_pad_all(bar, 0, 0);
	lv_obj_set_style_pad_left(bar, 1, 0);
	lv_obj_set_style_pad_right(bar, 1, 0);
	lv_obj_set_style_radius(bar, 0, 0);
	lv_obj_set_style_border_width(bar, 1, 0);
	lv_obj_set_style_border_side(bar, LV_BORDER_SIDE_TOP, 0);
	lv_obj_remove_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

	left_label = lv_label_create(bar);
	lv_obj_align(left_label, LV_ALIGN_LEFT_MID, 0, 0);

	right_label = lv_label_create(bar);
	lv_obj_align(right_label, LV_ALIGN_RIGHT_MID, 0, 0);

	ui_softkeys_set(NULL, NULL);
}

void ui_softkeys_set(const char *left, const char *right)
{
	lv_label_set_text(left_label, left != NULL ? left : "");
	lv_label_set_text(right_label, right != NULL ? right : "");
}
