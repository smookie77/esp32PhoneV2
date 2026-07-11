# esp32PhoneV2 — Zephyr Firmware Architecture

Status: draft for review — 2026-07-07
Scope: full rewrite of `SOFTWARE/FIRMWARE` (Arduino/PlatformIO) as a Zephyr application.
The Arduino tree stays in place as reference until feature parity.

## 1. Goals

1. **RAZR-style UX** on a 128×64 mono LCD: screen stack, two softkeys + center
   select, persistent status bar, telephony preempts everything.
2. **Scalable app model**: adding an app touches no core code — apps register
   themselves and receive events; they never own the main loop.
3. **SD card support** for both media files and **loadable apps**.
4. **Forward-compatible** with the next PCB revision (TFT + touch): LVGL and the
   input abstraction must survive that hardware change unmodified.

## 2. Platform decisions

| Concern | Decision |
|---|---|
| RTOS | Zephyr (pin a recent release in `west.yml`; verify LLEXT/Xtensa status at scaffold time) |
| Board | Custom board def `boards/smookie/esp32phone_v2` from day one |
| GUI | LVGL (Zephyr module), monochrome theme; 1-bit draw buffer |
| IPC | zbus channels for all cross-module communication |
| Config/persist | Zephyr settings subsystem on internal-flash LittleFS |
| Media/apps storage | SD card, FATFS |
| Debug | Zephyr shell over USB CDC + logging subsystem |
| Loadable apps | LLEXT (native ELF modules); see §8 for rationale and fallback |

## 3. Repository layout

```
SOFTWARE/ZEPHYR/
├── west.yml                  # workspace manifest, pins Zephyr + modules (LVGL)
├── app/
│   ├── CMakeLists.txt
│   ├── prj.conf              # + boards/*.conf overlays for size/debug variants
│   ├── src/
│   │   ├── main.c            # boot: services up → UI shell → home screen
│   │   ├── core/             # app framework: registry, lifecycle, screen stack
│   │   ├── services/         # input, modem, audio, power, storage
│   │   ├── ui/               # status bar, softkey bar, widget kit (LVGL)
│   │   └── apps/             # built-in apps: home, settings, music, messages, call
│   └── include/phone/        # public API headers (this is also the SD-app ABI)
├── boards/smookie/esp32phone_v2/   # devicetree, Kconfig, pinctrl
└── ARCHITECTURE.md           # this file
```

## 4. Layer diagram

```
┌─────────────────────────────────────────────────────┐
│ Apps: home, call, messages, settings, music, [SD…]  │  passive guests
├─────────────────────────────────────────────────────┤
│ App framework: registry · lifecycle · screen stack  │
│ UI shell: status bar · softkey bar · widget kit     │  one LVGL/UI thread
├────────────────────── zbus ─────────────────────────┤
│ Services (threads): input · modem · audio · power   │
│                     storage (SD hotplug, app scan)  │
├─────────────────────────────────────────────────────┤
│ Zephyr: LVGL · modem subsys · I2S · SDHC/FATFS      │
│         settings/LittleFS · shell · llext           │
├─────────────────────────────────────────────────────┤
│ Devicetree: ST7567(I2C) · TCA9555 · A7680C(UART)    │
│             MAX98357A/PCM1809(I2S) · SD-SPI · pwr   │
└─────────────────────────────────────────────────────┘
```

Rules that keep this honest:

- **Apps never touch drivers or services directly** — they call the `phone/` API
  and receive zbus events. (This is also what makes SD-loaded apps possible:
  the ABI surface is exactly `include/phone/`.)
- **Only the UI thread touches LVGL.** Services publish state; the shell and the
  foreground app render it.
- **Services never call apps.** They publish to zbus; the framework decides who
  is foreground and routes events.

## 5. zbus channel map

| Channel | Publisher | Payload (sketch) | Notes |
|---|---|---|---|
| `chan_input` | input service | `{key_id, action}` action ∈ press/release/long/repeat | keypad semantics resolved once, centrally |
| `chan_telephony` | modem service | `{event, number[]}` event ∈ incoming/connected/ended/missed | triggers call-screen preemption |
| `chan_sms` | modem service | `{slot_id}` (body read via API, not on the bus) | |
| `chan_cell_status` | modem service | `{rssi, registered, operator[]}` | status bar |
| `chan_power` | power service | `{soc_pct, charging, vbat_mv}` | status bar + low-batt dialog |
| `chan_audio` | audio service | `{state, source}` playing/paused/stopped | music app + status bar icon |
| `chan_storage` | storage service | `{event}` sd_inserted/sd_removed/apps_rescanned | menu refresh, media library |
| `chan_time` | time service (1/min) | `{epoch}` | status bar clock |

Key IDs are **semantic**, not physical: `KEY_SOFT_L`, `KEY_SOFT_R`, `KEY_SELECT`,
`KEY_UP/DOWN/LEFT/RIGHT`, `KEY_CALL`, `KEY_END`, `KEY_0`–`KEY_9`, `KEY_STAR`,
`KEY_HASH`. The TCA9555 matrix scan maps to these in the input service; the
future touchscreen maps gestures to the same events where sensible.

## 6. App framework

```c
/* include/phone/app.h — the contract; built-in and SD apps implement the same */
typedef struct phone_app_api {
    const char *name;                    /* menu label            */
    const void *icon;                    /* LVGL image descriptor */
    int  (*create) (struct app_ctx *);   /* allocate state        */
    void (*resume) (struct app_ctx *);   /* became foreground: build LVGL screen */
    void (*pause)  (struct app_ctx *);   /* lost foreground: release screen      */
    void (*destroy)(struct app_ctx *);
    bool (*on_event)(struct app_ctx *, const struct phone_event *); /* false = bubble up (e.g. Back) */
    uint32_t abi_version;                /* checked for SD apps   */
} phone_app_api_t;

#define PHONE_APP_DEFINE(api) /* iterable-section registration for built-ins */
```

- **Registry** has two sources: link-time iterable section (built-ins) and the
  storage service's SD scan (LLEXT). The home menu just iterates the registry —
  it cannot tell them apart.
- **Screen stack**: `screen_push(app)` / `screen_pop()`. Back key pops unless the
  app consumes it. Dialogs are stack entries too.
- **Preemption**: on `chan_telephony:incoming`, the framework pushes the call
  app above whatever is running (`pause`d, not destroyed) and pops back after.
- `app_ctx` carries per-app state pointer, its LVGL parent object, and the
  softkey-bar handle (each screen declares its softkey labels).

## 7. Services

Each service = one thread + a start/stop/status wrapper (your existing service
manager concept), exposed via the Zephyr shell: `svc status modem`, etc.

- **input** — TCA9555 matrix scan (interrupt-driven via expander INT line if
  wired, else 20 ms poll), debounce, long-press/repeat, publishes `chan_input`.
- **modem** — SIMCom A7680C (LTE Cat-1) on UART1 via Zephyr's modem subsystem; owns call state
  machine, SMS storage (PDU→LittleFS), network status. Publishes telephony/sms/
  cell-status channels; exposes `phone/modem.h` (dial, hangup, send_sms…).
- **audio** — I2S out to MAX98357A (incl. SD_MODE gpio), I2S in from PCM1809.
  Decoding (MP3 via a decoder module) in its own thread; playlist state.
  PSRAM for decode/ring buffers.
- **power** — battery voltage ADC, charge status, backlight/idle dimming, deep
  sleep policy.
- **storage** — mounts internal LittleFS at boot; SD detect/mount (FATFS at
  `/sd`), publishes hotplug events, scans `/sd/apps/` for loadable apps.

## 8. SD card support

**Files** (low risk): SDHC in SPI mode (or the S3 SDMMC host if the next PCB
routes it), FATFS mount at `/sd`. Layout:

```
/sd/music/…            # media
/sd/apps/<name>/app.llext   # relocatable ELF module
/sd/apps/<name>/manifest    # name, icon, abi_version, requested perms (future)
```

**Loadable apps** (the ambitious part) — decision: **LLEXT first, VM as fallback.**

- **LLEXT** (Zephyr's linkable loadable extensions): apps are C, compiled
  against `include/phone/` with the Zephyr SDK, loaded from SD into internal
  SRAM and linked at runtime against symbols we export (`EXPORT_SYMBOL` on the
  whole `phone/` API + the LVGL subset we bless). Pros: same language/API as
  built-in apps — any built-in app can be moved to SD as a test; no interpreter
  overhead. Risks, honestly stated:
  - Xtensa relocation support in LLEXT is newer than ARM's — must validate
    early on real hardware (this is Milestone M4's *first* task, not its last).
  - Code must live in instruction-capable internal SRAM — budget ~64–128 KB for
    a resident SD app; PSRAM is data-only for our purposes.
  - **ABI discipline**: `phone/` headers are the frozen surface; bump
    `abi_version` on any breaking change and refuse mismatched apps at load.
- **Fallback / future track**: a small VM (Lua or WAMR) bound to the same
  `phone/` API. Sandboxed and ABI-stable — the better long-term story if we
  ever want *other people* writing apps — but slower and a second API binding
  to maintain. The registry design means we can add it later without touching
  the framework.

## 9. Storage & memory budget

- **Internal flash (16 MB)**: code + LittleFS partition (settings, contacts,
  SMS) + spare partition reserved for future OTA slot-B.
- **PSRAM (8 MB)**: audio buffers, LVGL working memory, media library index.
- **Internal SRAM**: threads/stacks, LVGL 1-bit draw buffer (~1 KB for 128×64),
  LLEXT app code region (reserved, fixed-size heap so a leaky app can't take
  down the system).

## 10. Milestones

- **M0 — skeleton**: custom board def boots; display + keypad in devicetree;
  LVGL "hello"; shell on USB CDC. *Proves the board definition.* ✅ 2026-07
- **M1 — UI frame**: input service, zbus, screen stack, status bar, softkey
  bar, home menu with built-in app registry, settings app (ports the widget
  kit: list view, dialog). *Proves the whole interaction model.*
  Code complete (builds; `phone key <name>` shell cmd injects keys for
  testing over USB) — pending verification on hardware.
- **M2 — telephony**: modem service, cell status in bar, call app with
  preemption, SMS receive + messages app (text-entry widget, multi-tap).
- **M3 — media**: SD mount + FATFS, power service, audio out, music app.
- **M4 — loadable apps**: LLEXT spike (load a "hello" app from SD) → freeze
  `phone/` ABI → move one built-in app to SD as the reference example.

## 11. Hardware facts (verified against schematics, 2026-07-07)

Former open questions, now answered from `Mainboard_schematics.pdf` (PCB4) and
`Keyboard_schematic.pdf` (PCB3):

1. **SD slot exists on the mainboard** (TF1), wired in SPI mode:
   SCK=IO2, MOSI=IO43, MISO=IO44, CS=IO42. Because MOSI/MISO sit on the UART0
   pins, the **console must be USB-Serial-JTAG** (IO19/20 → USB-C).
2. TCA9555 **INT# is not routed** off the keyboard PCB (8-pin header carries
   only mic±, I2C, power) → keypad scan is polled (~20 ms). The expander also
   carries the **Vol+/Vol−/Power side buttons**, and the mic (ICS-40720) lives
   on the keyboard PCB.
3. **No battery-voltage divider found** — power service gets charge state only
   (BQ24090 `CHG_STATE`); add VBAT sense to the next-PCB wishlist.7
4. Full GPIO map: IO0=BOOT · IO3=amp SD_MODE · IO4=I2S LRC/FSYNC ·
   IO5=I2S BCLK · IO6=DAC DIN · IO7=ADC SDOUT · IO8=modem RST ·
   IO9/IO10=I2C SDA/SCL (ST7567 + TCA9555) · IO15/16=modem TX/RX ·
   IO17=modem RI · IO18=modem PWRKEY · IO19/20=USB. Spare: IO1, IO11–14, IO21,
   IO38–41, IO47/48 (EXT header).
5. Corrections to older docs: the modem is a **SIMCom A7680C-LANV** (not
   SIM800C/SIM7680C); the charger is a **BQ24090** (not TP4056); both I2S
   codecs share one bus (full duplex): amp + ADC on BCLK=IO5, LRC=IO4; the
   modem's analog audio out feeds PCM1809 IN2.
