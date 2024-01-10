#!/bin/sh
set -e

export BOARD=$1

case $BOARD in
    adafruit_feather_m0_basic_proto|arduino_due)
        west build -p -b ${BOARD} -- \
            -DEXTRA_CONF_FILE=$(pwd)/boards/adafruit_airlift.conf
        ;;
    *) # Invalid option
        echo "Error: Invalid option"
        exit 1
esac
