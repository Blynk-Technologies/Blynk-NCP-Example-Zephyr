/*
 * Copyright (c) 2024 Blynk Technologies Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/logging/log.h>

#include <blynk_ncp/blynk_ncp.h>

LOG_MODULE_REGISTER(blynk_example, CONFIG_LOG_DEFAULT_LEVEL);

void periodic_timer_work_handler(struct k_work *work)
{
    const uint32_t value = k_uptime_get_32();
    LOG_INF("Sending %u to Virtual Pin 2", value);

    char buff[16];
    snprintf(buff, sizeof(buff), "%u", value);
    rpc_buffer_t val = { (uint8_t*)buff, strlen(buff) };
    rpc_blynk_virtualWrite(2 /*VPin*/, val);
}

K_WORK_DEFINE(periodic_timer_work, periodic_timer_work_handler);

void periodic_timer_handler(struct k_timer*)
{
    k_work_submit(&periodic_timer_work);
}

K_TIMER_DEFINE(periodic_timer, periodic_timer_handler, NULL);

void rpc_client_blynkVPinChange_impl(uint16_t vpin, rpc_buffer_t param)
{
    switch (vpin)
    {
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

int main(void)
{
#if defined(CONFIG_USB_CDC_ACM)
    // Sleep to wait for a CDC connection
    k_msleep(3000);
#endif

    LOG_INF("Blynk.NCP host example on %s", CONFIG_BOARD);
    LOG_INF("Firmware version: %s", CONFIG_BLYNK_FIRMWARE_VERSION);

    // Mark OTA image as OK
    if (boot_set_confirmed_multi(0))
    {
        LOG_ERR("Can't confirm boot image");
    }

    // Initialize Blynk.NCP
    if (blynk_ncp_init())
    {
        LOG_ERR("Can't initialize Blynk.NCP");
        return 1;
    }

    // Set the configuration mode timeout to 30 minutes (for demo purposes)
    rpc_blynk_setConfigTimeout(30*60);

    // Start a periodic timer
    k_timer_start(&periodic_timer, K_SECONDS(1), K_SECONDS(1));

    k_sleep(K_FOREVER);

    return 0;
}

