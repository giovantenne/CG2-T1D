# WiFi Desktop glucose monitor device

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
- Upgradable firmware for future features and improvements

## User manual

This is a stand-alone device. This means that you don’t need a PC  to get it running and that you can simply power it through the usb-c port. Before doing so  you will need to connect it to your WiFi network by following these simple steps:

- Power-up your device. When setting it up for the first time, and everytime it is not able to connect to an existing WiFi network, the device will start in “access-point/captive-portal" mode and it will broadcast the WiFi SSID “T1D-Sucks”.
- Connect your phone/computer to the “T1D-Sucks” WiFi network using the following password: 12345678
- Your phone/computer should prompt you with a login/welcome page. If not you can simply browse to the following address: http://172.217.28.1. You should then be able to see and access the device setup interface.
- On the menu click “Configure new AP”. You should then be presented with a list of existing WiFi networks.
- Click on your network SSID name, enter your passphrase and click “Apply”.
- Browse to the QR code displayed on the screen to enter your CGM credentials (currently only Abbott Freestyle Libre 2 is supported)

You also can easily update the device firmware by downloading the latest version, if needed, from the following link: https://github.com/giovantenne/CG2-T1D/releases

You can check the firmware version you are currently running  by simply  looking at the display during the booting process, below the logo.


## Video
[![See the video here](https://www.datocms-assets.com/56675/1757099976-pxl_20250905_184210330.jpg?fm=webp&w=610)](https://www.youtube.com/watch?v=8-MzdPxjns4)


## Requirements
- TTGO T-Display - [here](https://www.lilygo.cc/products/lilygo%C2%AE-ttgo-t-display-1-14-inch-lcd-esp32-control-board)
- 3D BOX - [here](stl/)

## Download and load the firmware

The easiest way to flash firmware. Build your own CG2-T1D using the following firmware flash tool:

- Download the latest firmware from the [Releases](https://github.com/giovantenne/CG2-T1D/releases) page
- Download the [partitions.bin](https://github.com/giovantenne/CG2-T1D/raw/refs/heads/master/bin/partitions.bin) file
- Download the [bootloader.bin](https://github.com/giovantenne/CG2-T1D/raw/refs/heads/master/bin/bootloader.bin) file
- Open the [ESP Web Tool](https://esp.huhn.me/) to flash the firmware directly from your browser  (recommended via Google Chrome incognito mode)
- Connect your device, set the table as shown in the image below, and click _PROGRAM_:

![ESP Web Tool Flash table](https://github.com/giovantenne/CG2-T1D/blob/master/bin/ESPWebTool.png)

## Build from source and load the firmware
- Install [PlatformIO Core](https://platformio.org/install/cli)
- Connect the board via USB
- Clone this repository `git clone https://github.com/giovantenne/CG2-T1D`
- Run `cd CG2-T1D && pio run -t upload`

## Disclaimer

 - CG2-T1D is not intended to serve as or to replace the real-time display of CGM data of the primary device or standard blood glucose home monitoring. All therapeutic decisions, including those regarding calculating insulin or other drug dosages, should be based on blood glucose measurements obtained from a blood glucose meter and not on data from the CG2-T1D device.
- The CG2-T1D is not intended to analyze, interpret, or modify the CGM data received from the primary device.
- The CG2-T1D is not intended to replace self-monitoring practices as advised by a physician.
- THIS SOFTWARE MUST NOT BE USED TO MAKE MEDICAL DECISIONS
- There is no warranty for this software
- This software is not supported or endorsed by Abbott, Libre or any other party
