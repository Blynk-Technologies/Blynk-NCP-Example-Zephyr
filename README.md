
# Blynk.NCP

[![Issues](https://img.shields.io/github/issues/Blynk-Technologies/BlynkNcpExample_Zephyr_Zephyr.svg)](https://github.com/Blynk-Technologies/BlynkNcpExample_Zephyr/issues)
[![Downloads](https://img.shields.io/github/downloads/Blynk-Technologies/BlynkNcpDriver/total)](https://github.com/Blynk-Technologies/BlynkNcpDriver/releases/latest)
[![CI Status](https://img.shields.io/github/actions/workflow/status/Blynk-Technologies/BlynkNcpExample_Zephyr/build.yml?branch=main&logo=github&label=tests)](https://github.com/Blynk-Technologies/BlynkNcpExample_Zephyr/actions)
[![License](https://img.shields.io/github/license/Blynk-Technologies/BlynkNcpExample_Zephyr?color=blue)](LICENSE)
[![Stand With Ukraine](https://raw.githubusercontent.com/vshymanskyy/StandWithUkraine/main/badges/StandWithUkraine.svg)](https://stand-with-ukraine.pp.ua)

**Blynk.NCP** is a solution that off-loads connectivity to a **Network Co-Processor (NCP)** while your application logic resides on the **Primary MCU**. This implies a [dual-Microcontroller Unit (MCU)](https://docs.google.com/presentation/d/1aP2sQWB0J9EWj8Y1h5qeyfm2aFwaNSUKnCE-k7zxVnk/present) architecture.

<details><summary><b>When to use Blynk.NCP?</b></summary>

Using Blynk.NCP is recommended if one of these is true:

- You're building a new IoT product with specific requirements for the Primary MCU, and you're adding a separate connectivity module
- You are using Blynk for retrofitting your existing products
- You have included an **AT command**-based module, but you struggle to make it work right or to achieve your product goals
- You are looking for **ridiculously low** risks, integration efforts, and time to market, along with **improved reliability** of your products

</details>

<details><summary><b>Core Features</b></summary>

- **Blynk.Inject**: connect your devices easily using [<img src="https://cdn.rawgit.com/simple-icons/simple-icons/develop/icons/googleplay.svg" width="16" height="16" /> Android App](https://play.google.com/store/apps/details?id=cloud.blynk),
[<img src="https://cdn.rawgit.com/simple-icons/simple-icons/develop/icons/apple.svg" width="16" height="16" /> iOS App](https://apps.apple.com/us/app/blynk-iot/id1559317868) or [🌐 Web Dashboard](https://blynk.cloud)
  - `BLE`-assisted device provisioning for the best end-user experience
  - `WiFiAP`-based provisioning for devices without BLE support
  - **Network Manager**: WiFi (up to 16 saved networks), Ethernet, Cellular (depending on the hardware)
  - Advanced network connection troubleshooting
- Secure **Blynk.Cloud** connection that provides simple API for:
  - Data transfer with Virtual Pins, reporting Events, and accessing Metadata
  - `Time`, `Timezone` and `Location` with an ability to track local time when the device is offline, including DST transitions
- **Blynk.Air** - automatic Over The Air firmware updates using Web Dashboard
  - Both NCP and the Primary MCU firmware updates
  - Direct firmware upgrade using iOS/Android App before device activation

</details>

<details><summary><b>Extra Features</b></summary>

Additional services provided by the Blynk.NCP:

- `⏳ soon` Persistent automation scenarios - work even if the device is offline
- `⏳ soon` Non-volatile storage for the [Preferences](https://github.com/vshymanskyy/Preferences) library
- `✅ ready` NCP-assisted [fail-safe OTA updates](https://github.com/Blynk-Technologies/BlynkNcpDriver/blob/main/docs/Firmware%20Upgrade.md#ncp-assisted-fail-safe-ota-updates)
- `✅ ready` Connectivity-related **device state indication** - requires a monochrome/RGB/addressable LED attached to the NCP
- `✅ ready` **User button** (also used for configuration reset) - requires a momentary push button attached to the NCP
- `✅ ready` **Factory testing** and provisioning
- `🤔 later` Generic File System storage
- `🤔 later` Generic UDP/TCP/TLS socket API

</details>

## Getting started

This repository contains examples for multiple boards and use cases.
In general, getting started with Blynk.NCP on Zephyr will consist of:

1. Setting up Zephyr and dev environment
2. Uploading Blynk.NCP firmware to your NCP module
3. Assembling the board
4. Bulding and flashing the sample firmware
5. Using Blynk iOS/Android app to add your device to the Blynk Cloud

We have prepared a set of step-by-step tutorials for each use case:

- [ST Nucleo L4R5ZI + AirLift Shield](docs/samples/basic/ST_Nucleo_L4R5ZI_with_AirLift.md)
- [WeAct Black Pill F411CE + Witty Cloud](docs/samples/basic/WeAct_Black_Pill_with_Witty_Cloud.md)
- [Adafruit Feather STM32F405 + AirLift FeatherWing](docs/samples/basic/Adafruit_Feather_STM32F405_with_AirLift.md)
- [Raspberry Pi Pico + Pico-ESP8266 Shield](docs/samples/basic/Raspberry_Pi_Pico_with_ESP8266_Shield.md)
- [Adafruit Feather M0 Basic + AirLift FeatherWing](docs/samples/without_ota/Adafruit_Feather_M0_Basic_with_AirLift.md) *(without OTA)*
- [UNO R4 Minima + AirLift Shield](docs/samples/without_ota/Arduino_UNO_R4_Minima_with_AirLift.md) *(without OTA, Work-In-Progress)*

## Troubleshooting

Check out the [troubleshooting guide](docs/Troubleshooting.md)

## Disclaimer

> The community edition of Blynk.NCP is available for personal use and evaluation.  
> If you're interested in using Blynk.NCP for commercial applications, feel free to [contact Blynk][blynk_sales]. Thank you!


[blynk_sales]: https://blynk.io/en/contact-us-business

