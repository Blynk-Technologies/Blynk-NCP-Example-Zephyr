
# TODO + TODO NCP

## Prerequisites

Hardware:

- TODO
- TODO
- USB cable

Software:

- Latest official [Zephyr sources and SDK][zephyr_sdk]
- Blynk.NCP [firmware binary][blynk_ncp_binary]

## Flash the Network Co-Processor

👉 Follow the detailed [??? flashing guide](../../flashing_ncp/???.md)

## Assemble the board

TODO

## Configure and build the sample project

```sh
git clone https://github.com/Blynk-Technologies/BlynkNcpExample_Zephyr
cd BlynkNcpExample_Zephyr
git submodule update --init --recursive
```

Fill in [the information from your Blynk Template](https://bit.ly/BlynkInject):

```
cd samples/basic
echo 'CONFIG_BLYNK_TEMPLATE_ID="TMPxxxxxxxxx"' >> prj.conf
echo 'CONFIG_BLYNK_TEMPLATE_NAME="OurProduct"' >> prj.conf
```

Build:

```sh
export ZEPHYR_BASE=~/zephyrproject/zephyr
./build.sh TODO
```

<details><summary><b>Expected output</b></summary>

```log
TODO
```

</details>

## Flash the board

Use USB cable to connect the board to your PC.
TODO: Instructions on how to flash

```sh
# Flash MCUboot
west flash --build-dir build/mcuboot
# Flash sample
west flash --build-dir build/basic
```

TODO: Instructions on how to access the console (connect a UART adapter, etc.)
Use your favourite serial terminal software (`PuTTY`, `minicom`, `screen`) to access the serial console.
The expected serial monitor output looks like this:

```log
*** Booting Zephyr OS build zephyr-v3.5.0-3603-g603c3af895b0 ***
[00:00:03.002,000] <inf> blynk_example: Blynk.NCP host example
[00:00:03.002,000] <inf> blynk_example: Firmware version: 0.0.1
[00:00:03.854,000] <inf> blynk_lib: Blynk.NCP ready br 38400
[00:00:03.854,000] <inf> blynk_lib: setting target br 115200
[00:00:03.883,000] <inf> blynk_lib: Blynk.NCP ready br 115200
[00:00:03.886,000] <inf> blynk_lib: NCP firmware: 0.6.3
[00:00:03.903,000] <inf> blynk_lib: NCP state changed [Not Initialized] => [Configuration]
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

