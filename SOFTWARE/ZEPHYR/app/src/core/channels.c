#include <phone/channels.h>
#include <phone/keys.h>

ZBUS_CHAN_DEFINE(chan_input,
		 struct phone_key_event,
		 NULL,				/* validator */
		 NULL,				/* user data */
		 ZBUS_OBSERVERS_EMPTY,		/* observers attach via ZBUS_CHAN_ADD_OBS */
		 ZBUS_MSG_INIT(.key = KEY_NONE, .action = KEY_ACTION_RELEASE)
);
