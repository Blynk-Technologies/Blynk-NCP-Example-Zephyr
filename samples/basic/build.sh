#!/bin/sh
set -e

export BOARD=$1
shift

case $BOARD in
    nucleo_l4r5zi|adafruit_feather_stm32f405)
        west build -p -b ${BOARD} --sysbuild . -- \
            -Dmcuboot_DTC_OVERLAY_FILE=$(pwd)/boards/${BOARD}.overlay \
            -Dmcuboot_EXTRA_DTC_OVERLAY_FILE=${ZEPHYR_BASE}/../bootloader/mcuboot/boot/zephyr/app.overlay \
            -DEXTRA_CONF_FILE=$(pwd)/boards/adafruit_airlift.conf ${@}
        ;;
    blackpill_f411ce)
        west build -p -b ${BOARD} --sysbuild . -- \
            -Dmcuboot_DTC_OVERLAY_FILE=$(pwd)/boards/${BOARD}.overlay \
            -Dmcuboot_EXTRA_DTC_OVERLAY_FILE=${ZEPHYR_BASE}/../bootloader/mcuboot/boot/zephyr/app.overlay \
            -DEXTRA_CONF_FILE=$(pwd)/boards/witty_cloud.conf ${@}
        ;;
    rpi_pico)
        west build -p -b ${BOARD} --sysbuild . -- \
            -Dmcuboot_DTC_OVERLAY_FILE=$(pwd)/boards/${BOARD}.overlay \
            -Dmcuboot_EXTRA_DTC_OVERLAY_FILE=${ZEPHYR_BASE}/../bootloader/mcuboot/boot/zephyr/app.overlay ${@}
        ;;
    *) # Invalid option
        echo "Error: Invalid option"
        exit 1
esac
