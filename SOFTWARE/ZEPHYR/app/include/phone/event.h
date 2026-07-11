/*
 * Events delivered to the foreground app by the framework.
 * Grows union members as services land (telephony, storage, ...).
 */
#ifndef PHONE_EVENT_H_
#define PHONE_EVENT_H_

#include <phone/keys.h>

enum phone_event_type {
	PHONE_EVENT_KEY = 0,
};

struct phone_event {
	uint8_t type;		/* enum phone_event_type */
	union {
		struct phone_key_event key;
	};
};

#endif /* PHONE_EVENT_H_ */
