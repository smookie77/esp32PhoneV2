# esp32PhoneV2

A custom-built modular mobile phone based on the ESP32-S3 microcontroller. This project combines a high-performance ESP32-S3 with GSM connectivity, a monochrome LCD display, and a service-oriented firmware architecture.

## Features

- **Core**: ESP32-S3-DevKitC-1 with 16MB Flash and 8MB PSRAM (OPI).
- **Connectivity**:
  - **GSM/GPRS**: Integrated via SIM800C module for cellular communication.
  - **Wi-Fi & Bluetooth**: Leveraging native ESP32-S3 capabilities.
- **Display**: ST7567-based 128x64 monochrome LCD (using I2C).
- **Audio**: PCM1809 Audio ADC for high-quality audio input processing.
- **Input**: Custom keyboard PCB.
- **Firmware Architecture**: Service-based design allowing independent management (start/stop/status) of phone features like GSM, Display, Wi-Fi, and Keyboard.

## Project Structure

```text
├── HARDWARE/             # Hardware design and documentation
│   ├── ic_documentation/ # Datasheets (ESP32-S3, PCM1809, SIM800C)
│   └── pcb/              # PCB project files, BOMs, and Pick & Place files
└── SOFTWARE/
    └── FIRMWARE/         # ESP32 Source code (PlatformIO project)
        ├── src/          # Main application and service implementations
        ├── lib/          # Custom libraries (System services, Bitmaps)
        └── include/      # Global headers
```

## Hardware Components

- **MCU**: ESP32-S3 (ESP32-S3-WROOM-1 module recommended).
- **GSM Module**: SIM7680C.
- **Audio ADC**: PCM1809.
- **Display**: ST7567 128x64 LCD.
- **Keyboard**: Custom matrix-based keyboard (see `HARDWARE/pcb/Keyboard.epro`).

## Software Stack

- **Framework**: Arduino (within PlatformIO).
- **Graphics Library**: [U8g2](https://github.com/olikraus/u8g2) for the ST7567 display.
- **Service Manager**: A custom C-based service manager ([servicemgr.c](SOFTWARE/FIRMWARE/lib/sys/servicemgr.c)) used to handle background tasks and modular services.

## Getting Started

### Prerequisites

1.  **PlatformIO**: Install the PlatformIO IDE/Core.
2.  **Hardware**: Ensure you have the hardware assembled according to the PCB files in `HARDWARE/pcb/`.

### Building and Flashing

1.  Navigate to the firmware directory:
    ```bash
    cd SOFTWARE/FIRMWARE
    ```
2.  Build the project:
    ```bash
    pio run
    ```
3.  Upload to your ESP32-S3:
    ```bash
    pio run --target upload
    ```

### Monitoring

To view the service logs and debug information:
```bash
pio device monitor
```

## Current Status

- [x] Service manager implementation.
- [x] Basic UI with status bar and menu.
- [x] Bitmap integration.
- [ ] Full GSM integration (In progress).
- [ ] Hardware audio path validation.

## License

*(Specify your license here, e.g., MIT, GPLv3, etc.)*
