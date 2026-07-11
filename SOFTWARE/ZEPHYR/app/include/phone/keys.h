/*
 * Semantic key IDs — resolved once by the input service.
 * Apps never see matrix positions; a future touch UI maps to the same IDs.
 */
#ifndef PHONE_KEYS_H_
#define PHONE_KEYS_H_

#include <stdint.h>

enum phone_key {
	KEY_NONE = 0,
	KEY_0, KEY_1, KEY_2, KEY_3, KEY_4,
	KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
	KEY_STAR, KEY_HASH,
	KEY_SOFT_L, KEY_SOFT_R, KEY_SELECT,
	KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
	KEY_CALL, KEY_END,
	KEY_VOL_UP, KEY_VOL_DOWN, KEY_POWER,
	KEY_COUNT,
};

enum phone_key_action {
	KEY_ACTION_PRESS = 0,
	KEY_ACTION_RELEASE,
	KEY_ACTION_LONG,	/* fired once while still held */
	KEY_ACTION_REPEAT,	/* auto-repeat while held (nav keys) */
};

struct phone_key_event {
	uint8_t key;		/* enum phone_key */
	uint8_t action;		/* enum phone_key_action */
};

/* Lowercase shell-friendly name, e.g. "soft_l"; "?" if out of range. */
const char *phone_key_name(uint8_t key);

#endif /* PHONE_KEYS_H_ */
