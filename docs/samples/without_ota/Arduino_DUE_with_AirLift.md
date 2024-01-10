
# Arduino DUE + AirLift Shield

![main board](../../images/Arduino-Due.png)

> [!CRITICAL]
> 🚧 This sample is **Work-In-Progress** and is not fully functional yet. 🚧

> [!NOTE]
> This board is currently not supported by MCUboot on Zephyr,
> so OTA firmware updates using **Blynk.Air** won't work out of the box.

## Prerequisites

Hardware:

- Arduino DUE (SAM3X8E)
- Adafruit AirLift Shield (ESP32) to act as a **Network Co-Processor**
- USB to TTL Serial (3.3v) adapter or cable
- Micro-USB cable

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

```
cd samples/without_ota
echo 'CONFIG_BLYNK_TEMPLATE_ID="TMPxxxxxxxxx"' >> prj.conf
echo 'CONFIG_BLYNK_TEMPLATE_NAME="OurProduct"' >> prj.conf
```

Build:

```sh
export ZEPHYR_BASE=~/zephyrproject/zephyr
./build.sh arduino_due
```

<details><summary><b>Expected output</b></summary>

```log
...
...
[138/139] Linking C executable zephyr/zephyr.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:       33704 B       512 KB      6.43%
             RAM:       16896 B        96 KB     17.19%
        IDT_LIST:          0 GB         2 KB      0.00%
Generating files from /data/_Business/BlynkNcpExample_Zephyr/samples/without_ota/build/zephyr/zephyr.elf for board: arduino_due
[139/139] cd /data/_Business/BlynkNcpExample_Zephy...Zephyr/samples/without_ota/build/zephyr/zephyr.elf
```

</details>

## Flash the board

TODO

```sh
west flash
```

## Flash the Network Co-Processor

👉 Follow the detailed [AirLift Shield flashing guide](../../flashing_ncp/Adafruit_AirLift_Shield.md)

## Assemble the board and verify

> [!WARNING]
> When assembling the board, ensure that all USB ports are disconnected from any components, and that there is no power supply connected.

1. Insert AirLift Shield into the Arduino board.
2. Connect USB to TTL Serial adapter to the Arduino:

    | Arduino   | TTL adapter
    | :---      | :---
    | GND       | GND
    | PA11(TX)  | RX
    | PA10(RX)  | TX

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

- Use `west build -t menuconfig` to explore the available settings

## Troubleshooting

Check out the [troubleshooting guide](../../Troubleshooting.md)

## Disclaimer

> The community edition of Blynk.NCP is available for personal use and evaluation.
If you're interested in using Blynk.NCP for commercial applications, feel free to [contact Blynk][blynk_sales]. Thank you!

[zephyr_sdk]: https://docs.zephyrproject.org/latest/develop/getting_started/index.html
[blynk_ncp_binary]: https://docs.blynk.io/en/blynk.ncp/supported-connectivity-modules
[blynk_sales]: https://blynk.io/en/contact-us-business

