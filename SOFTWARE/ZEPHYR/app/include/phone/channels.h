/*
 * zbus channel map. All cross-module communication goes through here.
 */
#ifndef PHONE_CHANNELS_H_
#define PHONE_CHANNELS_H_

#include <zephyr/zbus/zbus.h>

/* struct phone_key_event — published by the input service */
ZBUS_CHAN_DECLARE(chan_input);

#endif /* PHONE_CHANNELS_H_ */
