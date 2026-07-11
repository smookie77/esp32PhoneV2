#include <zephyr/linker/iterable_sections.h>

/* built-in app registry (PHONE_APP_DEFINE) */
ITERABLE_SECTION_ROM(phone_app, Z_LINK_ITERABLE_SUBALIGN)
