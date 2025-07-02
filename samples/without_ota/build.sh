#!/bin/sh
set -e
BOARD=$1
ZEPHYR_VERSION=$2
if [ -z "$BOARD" ]; then
  echo "Usage: $0 <BOARD> [ZEPHYR_VERSION]"
  exit 1
fi
# Fallback to default version
if [ -z "$ZEPHYR_VERSION" ]; then
  ZEPHYR_VERSION="v0.0.0"
  shift 1
else
  shift 2
fi
echo "Building for board: $BOARD"
echo "Zephyr version: $ZEPHYR_VERSION"
version_gte() {
  [ "$(printf '%s\n%s' "$1" "$2" | sort -V | head -n1)" = "$2" ]
}

CONF_FILE=""
case $BOARD in
  adafruit_feather_m0_basic_proto|arduino_uno_r4_minima)
    CONF_FILE="$(pwd)/boards/adafruit_airlift.conf"
    ;;
  stm32f030_demo)
    CONF_FILE="$(pwd)/boards/stm32f030_demo.conf"

    # Backup original stm32f030_demo.conf if it doesn't exist
    if [ ! -f "boards/stm32f030_demo.conf.backup" ]; then
      cp boards/stm32f030_demo.conf boards/stm32f030_demo.conf.backup
    fi

    # Restore original stm32f030_demo.conf
    cp boards/stm32f030_demo.conf.backup boards/stm32f030_demo.conf

    # Add legacy configs for versions before 3.5.0
    if ! version_gte "$ZEPHYR_VERSION" "v3.5.0"; then
      echo "Adding legacy configuration options to stm32f030_demo.conf for Zephyr version < 3.5.0"
      echo "" >> boards/stm32f030_demo.conf
      echo "# Legacy configs for Zephyr < 3.5.0" >> boards/stm32f030_demo.conf
      echo "CONFIG_ISR_TABLES_LOCAL_DECLARATION=y" >> boards/stm32f030_demo.conf
      echo "CONFIG_LTO=y" >> boards/stm32f030_demo.conf
    else
      echo "Using clean stm32f030_demo.conf for Zephyr version >= 3.5.0 (no legacy configs)"
    fi
    ;;
esac

# Single build command with appropriate configuration
if [ -n "$CONF_FILE" ]; then
  echo "Using CONF_FILE: $CONF_FILE"
  west build -p -b "$BOARD" -- -DEXTRA_CONF_FILE="$CONF_FILE" -DKCONFIG_WARN_UNDEF_ASSIGN=n "$@"
else
  echo "No extra CONF_FILE used."
  west build -p -b "$BOARD" -- -DKCONFIG_WARN_UNDEF_ASSIGN=n "$@"
fi

# Restore original stm32f030_demo.conf after build (only if it was modified)
if [ "$BOARD" = "stm32f030_demo" ]; then
  echo "Restoring original stm32f030_demo.conf"
  cp boards/stm32f030_demo.conf.backup boards/stm32f030_demo.conf
  echo "Removing backup file"
  rm boards/stm32f030_demo.conf.backup
fi

