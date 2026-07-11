/* main() entry points into the framework — internal, not app-facing. */
#ifndef FRAMEWORK_H_
#define FRAMEWORK_H_

void framework_init(void);
void framework_run(void);	/* LVGL + event loop; never returns */

#endif /* FRAMEWORK_H_ */
