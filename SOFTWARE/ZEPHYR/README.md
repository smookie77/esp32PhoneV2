# esp32PhoneV2 — Zephyr firmware

See [ARCHITECTURE.md](ARCHITECTURE.md) for the design. This directory is a
west workspace: only `app/` (and these docs) are committed; `zephyr/`,
`modules/` and `.venv/` are fetched artifacts (see `.gitignore`).

## One-time setup

```bash
cd SOFTWARE/ZEPHYR
python3 -m venv .venv
.venv/bin/pip install west
.venv/bin/west init -l app
.venv/bin/west update --narrow -o=--depth=1
.venv/bin/pip install -r zephyr/scripts/requirements-base.txt
# Zephyr SDK with the ESP32-S3 toolchain (already at ~/zephyr-sdk-1.0.1):
#   <sdk>/setup.sh -t xtensa-espressif_esp32s3_zephyr-elf
```

## Build / flash / monitor

```bash
cd SOFTWARE/ZEPHYR
export ZEPHYR_SDK_INSTALL_DIR=~/zephyr-sdk-1.0.1

.venv/bin/west build -b esp32phone_v2/esp32s3/procpu app -d build
.venv/bin/west flash          # over the USB-C port (USB-Serial-JTAG)
.venv/bin/west espressif monitor
```

The console + shell are on the USB-C port (USB-Serial-JTAG), **not** UART0 —
the UART0 pins are wired to the SD card slot on this board.

## Custom board

`app/boards/smookie/esp32phone_v2/` defines the phone mainboard (PCB4):
ESP32-S3-WROOM-1U-N16R8, ST7567 LCD + TCA9555 keypad expander on I2C0
(SDA=IO9/SCL=IO10), A7680C modem on UART1, full-duplex I2S0
(MAX98357A + PCM1809), micro-SD on SPI2. The authoritative pin map is in the
header comment of `esp32phone_v2_procpu.dts` and in
`HARDWARE/pcb/Mainboard_schematics.pdf`.
