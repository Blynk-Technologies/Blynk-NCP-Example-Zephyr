/*
 * Copyright (c) 2024 Blynk Technologies Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <blynk_ncp/blynk_ncp.h>

LOG_MODULE_REGISTER(blynk_example, CONFIG_LOG_DEFAULT_LEVEL);

void rpc_client_blynkVPinChange_impl(uint16_t vpin, rpc_buffer_t param)
{
    switch (vpin) {
    case 0:
        /* VPin0 to VPin1 loopback */
        rpc_blynk_virtualWrite(1 /*VPin*/, param);
        break;
    case 1:
        /* VPin0 to VPin1 loopback */
        rpc_blynk_virtualWrite(0 /*VPin*/, param);
        break;
    default:
        LOG_WRN("unknown VPIN [%hu]", vpin);
        break;
    }
}

char *format_uint32(char *s, size_t len, uint32_t x)
{
    s += len;
    *--s = 0;
    if (!x) {
        *--s = '0';
    }
    for (; x; x /= 10) {
        *--s = '0' + (x % 10);
    }
    return s;
}

int main(void)
{
#if defined(CONFIG_USB_CDC_ACM)
    // Sleep to wait for a CDC connection
    k_msleep(3000);
#endif

    LOG_INF("Blynk.NCP host example on %s", CONFIG_BOARD);
    LOG_INF("Firmware version: %s", CONFIG_BLYNK_FIRMWARE_VERSION);

    // Initialize Blynk.NCP
    if (0 != blynk_ncp_init()) {
        LOG_ERR("Can't initialize Blynk.NCP");
        return 1;
    }

    while (BLYNK_STATE_CONNECTED != blynk_ncp_get_state()) {
        k_msleep(100);
    }

    while (true) {
        const uint32_t value = k_uptime_get_32();
        LOG_DBG("Sending %u to Virtual Pin 2", value);

        char buff[16];
        char *s = format_uint32(buff, sizeof(buff), value);
        rpc_buffer_t val = {(uint8_t *)s, strlen(s)};
        rpc_blynk_virtualWrite(2 /*VPin*/, val);

        k_msleep(1000);
    }

    return 0;
}
