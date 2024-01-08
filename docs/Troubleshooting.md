## General toubleshooting

- Try running the most basic Zephyr samples like [`blinky`][zephyr_blinky] or [`hello_world`][zephyr_hello].
- Some boards are not supported by `MCUboot` (and thus have no support for OTA updates). For those boards try using the `samples/without_ota` project.
- Some boards are using the `CDC_ACM` virtual port for debug output. If this doesn't work correctly, try switching `zephyr,console` to a hardware UART. You'll need an `USB to TTL Serial` cable or adapter to access the device console.
- If your board is unresponsive or misbehaving, it might be damaged. Please try the same procedure with a fresh new board.

## Cannot flash the board

- If your board exposes multiple USB ports, try each of them. Sometimes, connetors that are labeled as **native** are used for flashing.
- Check if your board is recognized by your PC and if the relevant drivers are loaded.
- Try rebooting your PC completely (by powering it off and on)

## OTA toubleshooting

- Zephyr has an [`MCUboot` example](zephyr_mcuboot), that should work out of the box. Try running it and see if your board boots correctly.
- When changing the partitions layout, calculate the offsets carefully. We've noticed that DTS compiler doesn't check partitions for overlapping, etc.
- If the OTA process fails, try lowering the uart baud rate (i.e. set `CONFIG_BLYNK_NCP_BAUD` to `115200` or less).


[zephyr_blinky]: https://docs.zephyrproject.org/latest/samples/basic/blinky/README.html
[zephyr_hello]: https://docs.zephyrproject.org/latest/samples/hello_world/README.html
[zephyr_mcuboot]: https://docs.zephyrproject.org/latest/samples/application_development/sysbuild/with_mcuboot/README.html

