# Installing Blynk.NCP on AirLift Shield

## Prerequisites

Hardware:

- [Adafruit AirLift Shield](https://www.adafruit.com/product/4285) (ESP32)
- USB to TTL Serial (3.3v) adapter or cable
- Jumper wires
- Soldering iron and accessories (read more [about soldering](https://learn.adafruit.com/adafruit-guide-excellent-soldering/tools))

Software:

- Espressif [esptool](https://docs.espressif.com/projects/esptool/en/)
- Blynk.NCP firmware binary: [BlynkNCP_generic_esp32_4M.flash.bin](https://github.com/blynkkk/BlynkNcpDriver/releases/latest/download/BlynkNCP_generic_esp32_4M.flash.bin)

## Assemble the shield

1. Solder the [Arduino header pins](https://learn.adafruit.com/adafruit-airlift-shield-esp32-wifi-co-processor/assembly-2) (the 2x3 ISP header is not needed)
2. Bridge the `TX(0)`, `RX(1)`, and `RST(5)` jumper pads (using small amount of solder)

![jumpers](../images/AirLift-Shield-Bottom.png)

## Connect the TTL Serial adapter or cable

| AirLift   | TTL adapter
| :---      | :---
| GND       | GND
| 5V        | 5V
| D5(RST)   | 3v3
| D0(TX)    | RX
| D1(RX)    | TX
| G0        | GND

> [!TIP]
> Utilize the prototyping area along with jumper wires for a more straightforward connection process

![connection](../images/AirLift-Shield-Top.png)

## Flash Blynk.NCP firmware

```sh
esptool.py --baud 460800 --before default_reset --after hard_reset write_flash --flash_size detect --erase-all 0x0 BlynkNCP_generic_esp32_4M.flash.bin
```

<details><summary><b>Expected esptool output</b></summary>

```log
TODO
```

</details>

## Verify

1. Use your favourite serial terminal software (`PuTTY`, `minicom`, `screen`) to access the TTL serial console (`38400 8N1`).
2. **Important:** Disconnect the `G0` from `GND`.
3. Power-cycle the shield by reconnecting the 5V supply and wait a few seconds.
4. The serial monitor should display:

    ```log
    [rpc port] Blynk.NCP started
    Version: x.x.x, Build: (date and time)
    ```

## Finish

Disconnect all the wires and adapters from the shield.

