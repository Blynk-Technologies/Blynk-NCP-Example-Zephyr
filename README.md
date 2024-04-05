
# Blynk.NCP integration for Zephyr OS

[![Issues](https://img.shields.io/github/issues/Blynk-Technologies/Blynk-NCP-Driver.svg)](https://github.com/Blynk-Technologies/Blynk-NCP-Driver/issues)
[![Downloads](https://img.shields.io/github/downloads/Blynk-Technologies/BlynkNcpDriver/total)](https://github.com/Blynk-Technologies/BlynkNcpDriver/releases/latest)
[![CI Status](https://img.shields.io/github/actions/workflow/status/Blynk-Technologies/Blynk-NCP-Example-Zephyr/build.yaml?branch=main&logo=github&label=tests)](https://github.com/Blynk-Technologies/Blynk-NCP-Example-Zephyr/actions)
[![License](https://img.shields.io/github/license/Blynk-Technologies/Blynk-NCP-Example-Zephyr?color=blue)](LICENSE)
[![Stand With Ukraine](https://raw.githubusercontent.com/vshymanskyy/StandWithUkraine/main/badges/StandWithUkraine.svg)](https://stand-with-ukraine.pp.ua)

**Blynk.NCP** is a solution that off-loads connectivity to a **Network Co-Processor (NCP)** while your application logic resides on the **Primary MCU**. This implies a [dual-Microcontroller Unit (MCU)](https://docs.google.com/presentation/d/1aP2sQWB0J9EWj8Y1h5qeyfm2aFwaNSUKnCE-k7zxVnk/present) architecture.

[**Read more about Blynk.NCP**](https://github.com/Blynk-Technologies/Blynk-NCP-Driver)

## Getting started

This repository contains examples for multiple boards and use cases.
In general, getting started with Blynk.NCP on Zephyr will consist of:

1. Setting up Zephyr and dev environment
2. Uploading Blynk.NCP firmware to your NCP module
3. Assembling the board
4. Building and flashing the sample firmware
5. Using Blynk iOS/Android app to add your device to the Blynk Cloud

We have prepared a set of step-by-step tutorials for each use case:

- [ST Nucleo L4R5ZI + AirLift Shield](docs/samples/basic/ST_Nucleo_L4R5ZI_with_AirLift.md)
- [WeAct Black Pill F411CE + Witty Cloud](docs/samples/basic/WeAct_Black_Pill_with_Witty_Cloud.md)
- [Adafruit Feather STM32F405 + AirLift FeatherWing](docs/samples/basic/Adafruit_Feather_STM32F405_with_AirLift.md)
- [Raspberry Pi Pico + Pico-ESP8266 Shield](docs/samples/basic/Raspberry_Pi_Pico_with_ESP8266_Shield.md)
- [Adafruit Feather M0 Basic + AirLift FeatherWing](docs/samples/without_ota/Adafruit_Feather_M0_Basic_with_AirLift.md) *(without OTA)*
- [Arduino UNO R4 Minima + AirLift Shield](docs/samples/without_ota/Arduino_UNO_R4_Minima_with_AirLift.md) *(without OTA, Work-In-Progress)*

## Troubleshooting

Check out the [troubleshooting guide](docs/Troubleshooting.md)

## Report an issue

Issues are maintained in the [Blynk NCP Driver](https://github.com/Blynk-Technologies/Blynk-NCP-Driver/issues) repository.

## Disclaimer

> The community edition of Blynk.NCP is available for personal use and evaluation.  
> If you're interested in using Blynk.NCP for commercial applications, feel free to [contact Blynk][blynk_sales]. Thank you!


[blynk_sales]: https://blynk.io/en/contact-us-business

