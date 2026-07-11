/* Framework-internal UI shell hooks — not part of the phone/ ABI. */
#ifndef UI_PRIV_H_
#define UI_PRIV_H_

#include <lvgl.h>

void ui_shell_create(lv_obj_t *screen);	/* status bar + content + softkey bar */
lv_obj_t *ui_content_area(void);	/* parent for app root containers */

void ui_statusbar_create(lv_obj_t *parent);
void ui_softkeybar_create(lv_obj_t *parent);

#endif /* UI_PRIV_H_ */
