/*
 * UI shell + widget kit. UI-thread only.
 */
#ifndef PHONE_UI_H_
#define PHONE_UI_H_

#include <lvgl.h>

/* EV 50 = the Arduino build's known-good setContrast(200) >> 2 */
#define PHONE_DISPLAY_DEFAULT_CONTRAST 50

/* Shell chrome */
void ui_softkeys_set(const char *left, const char *right);	/* NULL = blank */
void ui_statusbar_set_title(const char *title);

/*
 * List widget: vertical menu with an inverted selection bar.
 * The returned object is a plain lv_obj container of label rows.
 */
lv_obj_t *ui_list_create(lv_obj_t *parent);
lv_obj_t *ui_list_add(lv_obj_t *list, const char *text);
void ui_list_set_text(lv_obj_t *list, int index, const char *text);
void ui_list_select(lv_obj_t *list, int index);
void ui_list_nav(lv_obj_t *list, int delta);	/* move selection, clamped */
int  ui_list_selected(lv_obj_t *list);
int  ui_list_count(lv_obj_t *list);

/*
 * Modal dialog: message + right-softkey dismiss, above the app's UI.
 * The app owns dismissal: call ui_dialog_close() from its on_event.
 */
lv_obj_t *ui_dialog_open(const char *text);
void ui_dialog_close(lv_obj_t *dialog);

#endif /* PHONE_UI_H_ */
