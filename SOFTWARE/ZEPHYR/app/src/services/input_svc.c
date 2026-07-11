/*
 * Input service: polls the TCA9555 keypad matrix (INT# isn't routed off the
 * keyboard PCB), debounces, resolves long-press/repeat, publishes chan_input.
 *
 * Matrix wiring (Keyboard_schematic.pdf, PCB3): rows on expander pins 7-13
 * with external pull-ups, columns on pins 6/5/4, key closes row to column.
 * Idle detection drives all columns low so one port read shows any press.
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/shell/shell.h>
#include <zephyr/logging/log.h>
#include <phone/channels.h>
#include <phone/keys.h>

LOG_MODULE_REGISTER(input_svc, LOG_LEVEL_INF);

#define SCAN_PERIOD_MS	20
#define DEBOUNCE_SCANS	2	/* stable scans before a state change */
#define LONG_PRESS_MS	800
#define REPEAT_DELAY_MS	500
#define REPEAT_RATE_MS	150

#define NUM_ROWS 7
#define NUM_COLS 3

static const struct device *const expander = DEVICE_DT_GET(DT_NODELABEL(tca9555));

static const uint8_t row_pins[NUM_ROWS] = { 7, 8, 9, 10, 11, 12, 13 };
static const uint8_t col_pins[NUM_COLS] = { 6, 5, 4 };

/*
 * Physical layout (matches the Arduino keypad_driver key map):
 *   [SoftL] [ Up ] [SoftR]
 *   [Left ] [ OK ] [Right]
 *   [Call ] [Down] [ End ]
 *   [ 1-9, *0# ]
 * Side buttons (Vol+/Vol-/Power) also sit on this expander but their pins
 * aren't identified yet — map them here once confirmed at bring-up.
 */
static const uint8_t key_map[NUM_ROWS][NUM_COLS] = {
	{ KEY_SOFT_L, KEY_UP,     KEY_SOFT_R },
	{ KEY_LEFT,   KEY_SELECT, KEY_RIGHT  },
	{ KEY_CALL,   KEY_DOWN,   KEY_END    },
	{ KEY_1,      KEY_2,      KEY_3      },
	{ KEY_4,      KEY_5,      KEY_6      },
	{ KEY_7,      KEY_8,      KEY_9      },
	{ KEY_STAR,   KEY_0,      KEY_HASH   },
};

static const char *const key_names[KEY_COUNT] = {
	[KEY_NONE] = "none",
	[KEY_0] = "0", [KEY_1] = "1", [KEY_2] = "2", [KEY_3] = "3",
	[KEY_4] = "4", [KEY_5] = "5", [KEY_6] = "6", [KEY_7] = "7",
	[KEY_8] = "8", [KEY_9] = "9",
	[KEY_STAR] = "star", [KEY_HASH] = "hash",
	[KEY_SOFT_L] = "soft_l", [KEY_SOFT_R] = "soft_r", [KEY_SELECT] = "select",
	[KEY_UP] = "up", [KEY_DOWN] = "down",
	[KEY_LEFT] = "left", [KEY_RIGHT] = "right",
	[KEY_CALL] = "call", [KEY_END] = "end",
	[KEY_VOL_UP] = "vol_up", [KEY_VOL_DOWN] = "vol_down", [KEY_POWER] = "power",
};

const char *phone_key_name(uint8_t key)
{
	if (key >= KEY_COUNT || key_names[key] == NULL) {
		return "?";
	}
	return key_names[key];
}

/* Per-key hold bookkeeping, indexed by matrix slot (row * NUM_COLS + col) */
struct key_state {
	int64_t press_time;
	int64_t next_repeat;
	bool long_sent;
};

static struct key_state held[NUM_ROWS * NUM_COLS];

static bool key_repeats(uint8_t key)
{
	return key == KEY_UP || key == KEY_DOWN || key == KEY_LEFT || key == KEY_RIGHT;
}

static void publish(uint8_t key, uint8_t action)
{
	struct phone_key_event evt = { .key = key, .action = action };
	int err = zbus_chan_pub(&chan_input, &evt, K_MSEC(50));

	if (err != 0) {
		LOG_WRN("publish failed (%d)", err);
	}
}

static int set_cols(int active_col)
{
	for (int c = 0; c < NUM_COLS; c++) {
		int err = gpio_pin_set_raw(expander, col_pins[c],
					   (active_col < 0 || c == active_col) ? 0 : 1);
		if (err != 0) {
			return err;
		}
	}
	return 0;
}

static int read_rows(uint8_t *rows_low)
{
	gpio_port_value_t port;
	int err = gpio_port_get_raw(expander, &port);

	if (err != 0) {
		return err;
	}
	*rows_low = 0;
	for (int r = 0; r < NUM_ROWS; r++) {
		if ((port & BIT(row_pins[r])) == 0) {
			*rows_low |= BIT(r);
		}
	}
	return 0;
}

/* Returns a 21-bit pressed bitmap, or negative errno on bus trouble. */
static int32_t scan_matrix(void)
{
	uint8_t rows;
	int32_t pressed = 0;

	if (read_rows(&rows) != 0) {
		return -EIO;
	}
	if (rows == 0) {
		return 0;	/* idle: all columns already driven low */
	}

	/* Something is down — scan column by column to locate it. */
	for (int c = 0; c < NUM_COLS; c++) {
		if (set_cols(c) != 0 || read_rows(&rows) != 0) {
			set_cols(-1);
			return -EIO;
		}
		for (int r = 0; r < NUM_ROWS; r++) {
			if (rows & BIT(r)) {
				pressed |= BIT(r * NUM_COLS + c);
			}
		}
	}
	set_cols(-1);	/* back to all-low idle detection */
	return pressed;
}

static void process(int32_t stable, int32_t previous)
{
	int64_t now = k_uptime_get();

	for (int slot = 0; slot < NUM_ROWS * NUM_COLS; slot++) {
		uint8_t key = key_map[slot / NUM_COLS][slot % NUM_COLS];
		bool is_down = (stable >> slot) & 1;
		bool was_down = (previous >> slot) & 1;

		if (is_down && !was_down) {
			held[slot] = (struct key_state){
				.press_time = now,
				.next_repeat = now + REPEAT_DELAY_MS,
			};
			publish(key, KEY_ACTION_PRESS);
		} else if (!is_down && was_down) {
			publish(key, KEY_ACTION_RELEASE);
		} else if (is_down) {
			if (!held[slot].long_sent &&
			    now - held[slot].press_time >= LONG_PRESS_MS) {
				held[slot].long_sent = true;
				publish(key, KEY_ACTION_LONG);
			}
			if (key_repeats(key) && now >= held[slot].next_repeat) {
				held[slot].next_repeat = now + REPEAT_RATE_MS;
				publish(key, KEY_ACTION_REPEAT);
			}
		}
	}
}

static void input_thread(void *a, void *b, void *c)
{
	ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);

	if (!device_is_ready(expander)) {
		LOG_ERR("TCA9555 not ready — keypad disabled (shell key injection still works)");
		return;
	}

	for (int r = 0; r < NUM_ROWS; r++) {
		gpio_pin_configure(expander, row_pins[r], GPIO_INPUT);
	}
	for (int c = 0; c < NUM_COLS; c++) {
		gpio_pin_configure(expander, col_pins[c], GPIO_OUTPUT_LOW);
	}

	LOG_INF("keypad scan running (%d ms poll)", SCAN_PERIOD_MS);

	int32_t raw_prev = 0, stable = 0, stable_prev = 0;
	int agree = 0;

	for (;;) {
		k_msleep(SCAN_PERIOD_MS);

		int32_t raw = scan_matrix();

		if (raw < 0) {
			LOG_WRN("scan failed, retrying");
			k_msleep(500);
			continue;
		}

		/* debounce: accept only after DEBOUNCE_SCANS identical reads */
		agree = (raw == raw_prev) ? agree + 1 : 1;
		raw_prev = raw;
		if (agree >= DEBOUNCE_SCANS) {
			stable = raw;
		}

		process(stable, stable_prev);
		stable_prev = stable;
	}
}

K_THREAD_DEFINE(input_svc, 3072, input_thread, NULL, NULL, NULL, 5, 0, 0);

/* --- shell: inject keys over USB for UI testing without the keypad --- */

static int cmd_key(const struct shell *sh, size_t argc, char **argv)
{
	for (uint8_t k = KEY_0; k < KEY_COUNT; k++) {
		if (strcmp(argv[1], phone_key_name(k)) == 0) {
			publish(k, KEY_ACTION_PRESS);
			publish(k, KEY_ACTION_RELEASE);
			return 0;
		}
	}
	shell_error(sh, "unknown key '%s' (try: up down left right select soft_l soft_r call end 0-9 star hash)",
		    argv[1]);
	return -EINVAL;
}

SHELL_STATIC_SUBCMD_SET_CREATE(phone_cmds,
	SHELL_CMD_ARG(key, NULL, "Inject a key press, e.g. 'phone key select'",
		      cmd_key, 2, 0),
	SHELL_SUBCMD_SET_END
);
SHELL_CMD_REGISTER(phone, &phone_cmds, "Phone debug commands", NULL);
