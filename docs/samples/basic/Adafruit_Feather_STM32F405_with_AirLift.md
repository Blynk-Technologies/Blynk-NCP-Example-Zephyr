
# Adafruit Feather STM32F405 + AirLift FeatherWing

![main board](../../images/Adafruit-Feather-STM32F405.png)

## Prerequisites

Hardware:

- Adafruit Feather STM32F405
- Adafruit AirLift FeatherWing (ESP32) to act as a **Network Co-Processor**
- Jumper wires
- USB Type C cable

Software:

- Latest official [Zephyr sources and SDK][zephyr_sdk]
- Blynk.NCP [firmware binary][blynk_ncp_binary] (more on that later)

## Configure and build the sample project

```sh
git clone https://github.com/Blynk-Technologies/BlynkNcpExample_Zephyr
cd BlynkNcpExample_Zephyr
git submodule update --init --recursive
```

Fill in [the information from your Blynk Template](https://bit.ly/BlynkInject):

```sh
cd samples/basic
echo 'CONFIG_BLYNK_TEMPLATE_ID="TMPxxxxxxxxx"' >> prj.conf
echo 'CONFIG_BLYNK_TEMPLATE_NAME="OurProduct"' >> prj.conf
```

Build:

```sh
export ZEPHYR_BASE=~/zephyrproject/zephyr
./build.sh adafruit_feather_stm32f405
```

<details><summary><b>Expected output</b></summary>

```log
...
...
[173/174] Linking C executable zephyr/zephyr.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:       63144 B     389760 B     16.20%
             RAM:         23 KB       128 KB     17.97%
             CCM:          0 GB        64 KB      0.00%
        IDT_LIST:          0 GB         2 KB      0.00%
Generating files from build/basic/zephyr/zephyr.elf for board: adafruit_feather_stm32f405
image.py: sign the payload
image.py: sign the payload
image.py: sign the payload
image.py: sign the payload
[174/174] cd ....../build/basic/zephyr/zephyr.elf
[11/16] Performing build step for 'mcuboot'
[1/273] Preparing syscall dependency handling

[3/273] Generating include/generated/version.h
-- Zephyr version: 3.5.99 (/home/user/zephyrproject/zephyr), build: zephyr-v3.5.0-3889-ge49d174be910
[272/273] Linking C executable zephyr/zephyr.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:       31942 B        64 KB     48.74%
             RAM:       18304 B       128 KB     13.96%
             CCM:          0 GB        64 KB      0.00%
        IDT_LIST:          0 GB         2 KB      0.00%
Generating files from build/mcuboot/zephyr/zephyr.elf for board: adafruit_feather_stm32f405
[273/273] cd ....../build/mcuboot/zephyr/zephyr.elf
[16/16] Completed 'mcuboot'
```

</details>

## Flash the board

To flash STM32F405, you need to connect `B0` to `3v3` using a green (important!) jumper wire, then run these commands:

```sh
# Flash MCUboot
west flash --build-dir build/mcuboot
# Flash sample
west flash --build-dir build/basic
```

> [!NOTE]
> The flash utility will ask to `reset your board to switch to DFU mode`.  
> Press the `Rst` button shortly.

After flashing the board, please disconnect the jumper.

## Flash the Network Co-Processor

👉 Follow the detailed [AirLift FeatherWing flashing guide](../../flashing_ncp/Adafruit_AirLift_FeatherWing.md)

## Assemble the board and verify

> [!WARNING]
> When assembling the board, ensure that all USB ports are disconnected from any components, and that there is no power supply connected.

1. Insert AirLift FeatherWing shield into the Feather board.
2. Connect your device using USB. The device will appear as a `CDC-ACM` serial.
3. Use your favourite serial terminal software (`PuTTY`, `minicom`, `screen`) to access the serial console (`115200 8N1`).
4. The expected serial monitor output looks like this:

    ```log
    *** Booting Zephyr OS build zephyr-v3.5.0-3889-ge49d174be910 ***
    [00:00:03.002,000] <inf> blynk_example: Blynk.NCP host example
    [00:00:03.002,000] <inf> blynk_example: Firmware version: 0.0.1
    [00:00:03.854,000] <inf> blynk_ncp: Blynk.NCP ready br 38400
    [00:00:03.854,000] <inf> blynk_ncp: setting target br 115200
    [00:00:03.883,000] <inf> blynk_ncp: Blynk.NCP ready br 115200
    [00:00:03.886,000] <inf> blynk_ncp: NCP firmware: 0.6.3
    [00:00:03.903,000] <inf> blynk_ncp: NCP state changed [Not Initialized] => [Configuration]
    ```

## Use the Blynk iOS/Android app to configure your new device

Ensure that the Blynk App is installed on your smartphone.

Open the `Blynk App` -> click `Add New Device` -> select `Find Devices Nearby`


## Next steps

- Use [**Blynk.Air**](https://docs.blynk.io/en/blynk.console/blynk.air) to peform OTA update of your device firmware
  - The firmware file can be found here: **`./build/basic/zephyr/zephyr.signed.bin`**
- Learn about [Zephyr firmware signing](https://docs.zephyrproject.org/latest/develop/west/sign.html)
- Use `west build -t menuconfig` to explore the available settings

## Troubleshooting

Check out the [troubleshooting guide](../../Troubleshooting.md)

## Disclaimer

> The community edition of Blynk.NCP is available for personal use and evaluation.
If you're interested in using Blynk.NCP for commercial applications, feel free to [contact Blynk][blynk_sales]. Thank you!

[zephyr_sdk]: https://docs.zephyrproject.org/latest/develop/getting_started/index.html
[blynk_ncp_binary]: https://docs.blynk.io/en/blynk.ncp/supported-connectivity-modules
[blynk_sales]: https://blynk.io/en/contact-us-business

