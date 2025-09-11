# WiFi Desktop Glucose Monitor (CG2‑T1D)

This is a **free and open source project** that lets you check your CGM-detected glucose values in real time with a small piece of hardware.

This is a remix of my other project CryptoGadgets v2 (CG2) that I created for my personal use. You can find the original project [here](https://github.com/giovantenne/CG2)

![WiFi Desktop glucose monitor device](https://www.datocms-assets.com/56675/1757097907-pxl_20250905_170517849.jpg?fm=webp&w=610)


## Features

- Built-in WiFi – works independently, no PC required
- Real-time glucose level display
- 3 zoom levels to view the past 12, 6, or 3 hours
- Adjustable brightness for any environment
- Automatic refresh every 60 seconds
- Missing data visual-alert if no updates are received for more than 5 minutes
- Upgradable firmware (OTA check on boot)

## User Manual

This is a stand-alone device. This means that you don’t need a PC  to get it running and that you can simply power it through the usb-c port. Before doing so  you will need to connect it to your WiFi network by following these simple steps:

- Power-up your device. When setting it up for the first time, and everytime it is not able to connect to an existing WiFi network, the device will start in “access-point/captive-portal" mode and it will broadcast the WiFi SSID “T1D-Sucks”.
- Connect your phone/computer to the “T1D-Sucks” WiFi network using the following password: 12345678
- Your phone/computer should prompt you with a login/welcome page. If not you can simply browse to the following address: http://172.217.28.1. You should then be able to see and access the device setup interface.
- On the menu click “Configure new AP”. You should then be presented with a list of existing WiFi networks.
- Click on your network SSID name, enter your passphrase and click “Apply”.
- Browse to the QR code displayed on the screen to enter your CGM credentials (currently only Abbott Freestyle Libre 2 is supported). Enter:
  - Email and password for LibreView
  - Patient index (0 for first, 1 for second…)

You also can easily update the device firmware by downloading the latest version, if needed, from the following link: https://github.com/giovantenne/CG2-T1D/releases

You can check the firmware version you are currently running  by simply  looking at the display during the booting process, below the logo.

On every successful Wi‑Fi connection the device performs a lightweight OTA check; if a newer build is available on the update server it will show an "update in progress" screen and apply it automatically.

### Factory reset (wipe Wi‑Fi and app settings)

- Hold Button 1 (the top button, GPIO35) while powering up: the device will erase all saved Wi‑Fi networks and reset internal settings, then reboot into Access Point mode.
- Or browse to the IP shown during the boot process, go to the “Reset” page and confirm. This also erases known Wi‑Fi credentials.

After reset, the device will no longer reconnect to your previous network until you reconfigure it.


### Buttons (quick reference)

- Button 1 short press: cycle brightness
- Button 1 long press (~2s): force refresh
- Button 2 short press: toggle zoom (3 → 6 → 12 → hours)
- Button 2 long press (~2s): deep sleep

## Video
[![See the video here](https://www.datocms-assets.com/56675/1757099976-pxl_20250905_184210330.jpg?fm=webp&w=610)](https://www.youtube.com/watch?v=8-MzdPxjns4)


## Requirements
- TTGO T-Display - [here](https://www.lilygo.cc/products/lilygo%C2%AE-ttgo-t-display-1-14-inch-lcd-esp32-control-board)
- 3D BOX - [here](stl/)
- 3.7V 1100mAh LiPo Battery with Micro JST 1.25 connector (optional)

## Download and Load the Firmware

The easiest way to flash firmware. Build your own CG2-T1D using the following firmware flash tool:

- Download the latest firmware from the [Releases](https://github.com/giovantenne/CG2-T1D/releases) page
- Download the [partitions.bin](https://github.com/giovantenne/CG2-T1D/raw/refs/heads/master/bin/partitions.bin) file
- Download the [bootloader.bin](https://github.com/giovantenne/CG2-T1D/raw/refs/heads/master/bin/bootloader.bin) file
- Open the [ESP Web Tool](https://esp.huhn.me/) to flash the firmware directly from your browser  (recommended via Google Chrome incognito mode)
- Connect your device, set the table as shown in the image below, and click _PROGRAM_:

![ESP Web Tool Flash table](https://raw.githubusercontent.com/giovantenne/CG2-T1D/refs/heads/master/bin/ESPWebTool.png)

## Build From Source and Load the Firmware
- Install [PlatformIO Core](https://platformio.org/install/cli)
- Connect the board via USB
- Clone this repository `git clone https://github.com/giovantenne/CG2-T1D`
- Run `cd CG2-T1D && pio run -t upload`

### Run the test suite

This project includes on‑device unit tests (Unity) for core helpers and persistence.

- Build and run tests on the board:

  - `pio test -e ttgo-t1 -v`

Notes:
- Tests are isolated from the main app logic — the application entry points are guarded with `#ifndef UNIT_TEST`.
- The test runner resets the board between suites; each test takes ~20s to flash and run.

## Project Structure (developer overview)

- `src/app_state.cpp` and `include/app_state.h` — global device state and externs (display, Wi‑Fi portal, runtime/config values)
- `src/display.cpp` and `include/display.h` — all rendering (fonts, icons, ticker/graph, status screens)
- `src/api.cpp` and `include/api.h` — LibreView HTTP client, JSON parsing into `glucoseDoc`
- `src/config.cpp` and `include/config.h` — EEPROM persistence helpers and validation
- `src/config_store.cpp` and `include/config_store.h` — single point of mutation + persist for credentials and brightness
- `src/app_store.cpp` and `include/app_store.h` — runtime mutation helpers (glucose values, trend, battery, points)
- `src/portal.cpp` and `include/portal.h` — captive portal, handlers, Wi‑Fi reset (erases NVS + credentials)
- `src/buttons.cpp` and `include/buttons.h` — button behaviors (brightness/zoom, deep sleep, force refresh)
- `src/hardware.cpp` and `include/hardware.h` — battery reading and sleep delay
- `src/ota.cpp` and `include/ota.h` — OTA check and apply
- `include/board.h` — board pinout and constants



## Disclaimer

 - CG2-T1D is not intended to serve as or to replace the real-time display of CGM data of the primary device or standard blood glucose home monitoring. All therapeutic decisions, including those regarding calculating insulin or other drug dosages, should be based on blood glucose measurements obtained from a blood glucose meter and not on data from the CG2-T1D device.
- The CG2-T1D is not intended to analyze, interpret, or modify the CGM data received from the primary device.
- The CG2-T1D is not intended to replace self-monitoring practices as advised by a physician.
- THIS SOFTWARE MUST NOT BE USED TO MAKE MEDICAL DECISIONS
- There is no warranty for this software
- This software is not supported or endorsed by Abbott, Libre or any other party
