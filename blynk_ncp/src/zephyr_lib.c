/*
 * Copyright (c) 2024 Blynk Technologies Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/posix/unistd.h>
#include <zephyr/kernel.h>
#include <zephyr/linker/linker-defs.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(blynk_lib, CONFIG_BLYNK_LOG_LVL);

#include <blynk_ncp/blynk_ncp.h>
#include <BlynkRpcClient.h>

#ifndef BLYNK_THREAD_PRIO
#warning BLYNK_THREAD_PRIO not configured
#define BLYNK_THREAD_PRIO     (CONFIG_NUM_PREEMPT_PRIORITIES - 1)
#endif

#ifndef BLYNK_THERAD_STACK_SIZE
#warning BLYNK_THERAD_STACK_SIZE not configured
#define BLYNK_THERAD_STACK_SIZE          (512)
#endif

#define BLYNK_FIRMWARE_BUILD_TIME __DATE__ " " __TIME__


#if DT_HAS_CHOSEN(blynk_ncp_status_led)
static const struct gpio_dt_spec staus_led = GPIO_DT_SPEC_GET(DT_CHOSEN(blynk_ncp_status_led), gpios);
#endif

#if DT_HAS_CHOSEN(blynk_ncp_rst)
static const struct gpio_dt_spec ncp_rst = GPIO_DT_SPEC_GET(DT_CHOSEN(blynk_ncp_rst), gpios);
#endif

K_THREAD_STACK_DEFINE(ncp_stack_area, BLYNK_THERAD_STACK_SIZE);
static struct k_thread ncp_thread_data;
static k_tid_t ncp_tid;

#define BLYNK_PARAM_KV(k, v)    k "\0" v "\0"

K_MUTEX_DEFINE(ncp_rpc_mutex);

void uart_init(void);          // see zephyr_uart.c
int uart_set_br(uint32_t br);  // see zephyr_uart.c
void ota_run(void);            // see zephyr_ota.c

void ncpMutexLock(void)
{
    k_mutex_lock(&ncp_rpc_mutex, K_FOREVER);
}

void ncpMutexUnlock(void)
{
    k_mutex_unlock(&ncp_rpc_mutex);
}

static bool ncpSetupSerial(uint32_t timeout)
{
    const uint32_t br[] = {1200, 2400, 4800, 9600, 19200, 28800, 38400, 57600, 76800, 115200, 230400, 460800, 576000, 921600, CONFIG_BLYNK_NCP_BAUD};
    int i = 0;

    uart_init();
    RpcUartFraming_init();
    do
    {
        const int rc = uart_set_br(br[i]);
        if(rc)
        {
            LOG_ERR("can't set baudrate %d rc %d", br[i], rc);
            return false;
        }
        else
        {
            if (RPC_STATUS_OK != rpc_ncp_ping())
            {
                LOG_INF("NCP not responding br %d", br[i]);
                i = i+1 >= sizeof(br)/sizeof(br[0]) ? 0 : i+1;
                timeout -= (20 > timeout ? timeout : 20);
                k_msleep(20);
            }
            else
            {
                LOG_INF("Blynk.NCP ready br %d", br[i]);
                if(CONFIG_BLYNK_NCP_BAUD == br[i])
                {
                    return true;
                }
                LOG_INF("setting target br %d", CONFIG_BLYNK_NCP_BAUD);

                rpc_hw_setUartBaudRate(CONFIG_BLYNK_NCP_BAUD);
                k_msleep(20);

                if(uart_set_br(CONFIG_BLYNK_NCP_BAUD))
                {
                    LOG_ERR("can't set baudrate %d", CONFIG_BLYNK_NCP_BAUD);
                    return false;
                }
                if (RPC_STATUS_OK != rpc_ncp_ping())
                {
                    LOG_ERR("Error NCP not responding br %d", CONFIG_BLYNK_NCP_BAUD);
                    return false;
                }
                else
                {
                    LOG_INF("Blynk.NCP ready br %d", CONFIG_BLYNK_NCP_BAUD);
                }
                return true;
            }
        }
    }
    while(timeout);

    return false;
}

void ncpThread(void*, void*, void*)
{
    while(1)
    {
        rpc_run();

#if defined(CONFIG_BOOTLOADER_MCUBOOT)
        ota_run();
#endif

#if DT_HAS_CHOSEN(blynk_ncp_status_led)
        /* heartbeat led blink */
        static uint32_t t = 0;
        const uint32_t ct = k_uptime_get_32();
        if (ct - t > 250)
        {
            t = ct;
            gpio_pin_toggle_dt(&staus_led);
        }
#endif
        k_msleep(1);
    }
}

static void ncpReinitHandler(struct k_work *work)
{
    LOG_INF("NCP lib reiniting");
    k_thread_abort(ncp_tid);
    blynk_ncp_init();
}

K_WORK_DEFINE(ncpReinitWork, ncpReinitHandler);

void ncpReinitTimerHandler(struct k_timer *dummy)
{
    //timer called from system ISR so use work queue here to restart NCP later
    k_work_submit(&ncpReinitWork);
}

K_TIMER_DEFINE(ncpReinitTimer, ncpReinitTimerHandler, NULL);

static void ncpPingHandler(struct k_work *work)
{
    static int err_num = 0;
    if (RPC_STATUS_OK != rpc_ncp_ping())
    {
        if(err_num < 3)
        {
            err_num++;
            LOG_INF("NCP ping error. attempt [%d]", err_num);
        }
        else
        {
            LOG_INF("NCP ping error. rebooting NCP");
            err_num = 0;

#if DT_HAS_CHOSEN(blynk_ncp_rst)
            LOG_INF("NCP: perform hard reaset");
            gpio_pin_configure_dt(&ncp_rst, GPIO_OUTPUT_ACTIVE);
            k_msleep(100);
            gpio_pin_configure_dt(&ncp_rst, GPIO_OUTPUT_INACTIVE);
            k_msleep(100);
#endif
            k_timer_start(&ncpReinitTimer, K_SECONDS(1), K_NO_WAIT);
        }
    }
    else
    {
        if(err_num)
        {
            LOG_INF("NCP ping success");
        }
        err_num = 0;
    }
}

K_WORK_DEFINE(ncpPingtWork, ncpPingHandler);

void ncpPingTimerHandler(struct k_timer *dummy)
{
    //timer called from system ISR so use work queue here to ping NCP later
    k_work_submit(&ncpPingtWork);
}

K_TIMER_DEFINE(ncpPingTimer, ncpPingTimerHandler, NULL);

static const char* ncpGetStateString(uint8_t state)
{
    switch (state)
    {
        case BLYNK_STATE_IDLE             : return "Idle";
        case BLYNK_STATE_CONFIG           : return "Configuration";
        case BLYNK_STATE_CONNECTING_NET   : return "Connecting Network";
        case BLYNK_STATE_CONNECTING_CLOUD : return "Connecting Cloud";
        case BLYNK_STATE_CONNECTED        : return "Connected";

        case BLYNK_STATE_NOT_INITIALIZED  : return "Not Initialized";
        case BLYNK_STATE_OTA_UPGRADE      : return "NCP Upgrade";
        case BLYNK_STATE_ERROR            : return "Error";

        default                           : return "Unknown";
    }
}

void rpc_client_blynkStateChange_impl(uint8_t state)
{
    static uint8_t prv = BLYNK_STATE_NOT_INITIALIZED;
    if(prv == state)
    {
        // nothing changed
        return;
    }

    LOG_INF("NCP state changed [%s] => [%s]", ncpGetStateString(prv), ncpGetStateString(state));
    prv = state;
    switch(state)
    {
        case BLYNK_STATE_NOT_INITIALIZED:
            // reinit in 1 second
            k_timer_start(&ncpReinitTimer, K_SECONDS(1), K_NO_WAIT);
            break;
        default:
            break;
    }
}

// Handle various NCP events
void rpc_client_processEvent_impl(uint8_t event)
{
    switch ((RpcEvent)event) {
        /*
         * System events
         */
        case RPC_EVENT_NCP_REBOOTING:
            LOG_INF("NCP is rebooting. reinitialize NCP");
            /* start a one-shot timer */
            k_timer_start(&ncpReinitTimer, K_SECONDS(1), K_NO_WAIT);
            break;
        case RPC_EVENT_BLYNK_PROVISIONED:
            LOG_INF("NCP finished provisioning");
            break;
        case RPC_EVENT_BLYNK_TIME_SYNC:
            LOG_INF("NCP requests time sync from external time source");
            break;
        case RPC_EVENT_BLYNK_TIME_CHANGED: break;
            LOG_INF("NCP local time changed");
            break;
        /*
         * User button events (see rpc_hw_initUserButton)
         */
        case RPC_EVENT_HW_USER_CLICK:
            LOG_INF("NCP: user button click");
            break;
        case RPC_EVENT_HW_USER_DBLCLICK:
            LOG_INF("NCP: user button double click");
            break;
        case RPC_EVENT_HW_USER_LONGPRESS:
            LOG_INF("NCP: user button long press start");
            break;
        case RPC_EVENT_HW_USER_LONGRELEASE:
            LOG_INF("NCP: user button long press stop");
            break;
        case RPC_EVENT_HW_USER_RESET_START:
            LOG_INF("NCP: Button is pressed for 10 seconds => release to clear configuration");
            break;
        case RPC_EVENT_HW_USER_RESET_CANCEL:
            LOG_INF("NCP: Button is pressed for 15 seconds => cancel config reset operation");
            break;
        case RPC_EVENT_HW_USER_RESET_DONE:
            LOG_INF("NCP: Button was released => configuration is reset");
            break;

        default:
            LOG_INF("NCP: unknown event [%hhu]. doing nothing",event);
            break;
    }
}

int blynk_ncp_init(void)
{
    k_timer_stop(&ncpReinitTimer);
    k_timer_stop(&ncpPingTimer);

#if DT_HAS_CHOSEN(blynk_ncp_status_led)
    gpio_pin_configure_dt(&staus_led, GPIO_OUTPUT_INACTIVE);
#endif
#if DT_HAS_CHOSEN(blynk_ncp_rst)
    gpio_pin_configure_dt(&ncp_rst, GPIO_OUTPUT_INACTIVE);
    k_msleep(10);
#endif
    if(!strlen(CONFIG_BLYNK_TEMPLATE_ID))
    {
        LOG_ERR("CONFIG_BLYNK_TEMPLATE_ID not specified");
        return -1;
    }

    if(!strlen(CONFIG_BLYNK_TEMPLATE_NAME))
    {
        LOG_ERR("CONFIG_BLYNK_TEMPLATE_NAME not specified");
        return -1;
    }

    if(!strlen(CONFIG_BLYNK_FIRMWARE_VERSION))
    {
        LOG_ERR("CONFIG_BLYNK_FIRMWARE_VERSION not specified");
        return -1;
    }

    if (!ncpSetupSerial(5000))
    {
        LOG_WRN("can't setup serial");
        return -1;
    }

    const char* ncpFwVer = "unknown";
    if (!rpc_blynk_getNcpVersion(&ncpFwVer))
    {
        LOG_WRN("can't get NCP firmware version");
        return -1;
    }

    LOG_INF("NCP firmware: %s", ncpFwVer);

    // TODO: rpc_blynk_setConfigTimeout(30*60);


    /*
     * Embed the info tag into the MCU firmware binary
     * This structure is used to identify the firmware type
     * and version during the OTA upgrade
     */
    volatile const char firmwareTag[] = "blnkinf\0"
        BLYNK_PARAM_KV("mcu"    , CONFIG_BLYNK_FIRMWARE_VERSION)       // Primary MCU: firmware version
        BLYNK_PARAM_KV("fw-type", CONFIG_BLYNK_TEMPLATE_ID)            // Firmware type (usually same as Template ID)
        BLYNK_PARAM_KV("build"  , __DATE__ " " __TIME__)               // Firmware build date and time
        BLYNK_PARAM_KV("blynk"  , BLYNK_RPC_LIB_VERSION)               // Version of the NCP driver library
        "\0";
    (void)firmwareTag;

    //LOG_HEXDUMP_INF((void*)firmwareTag, sizeof(firmwareTag), "firmwareTag is:");


    rpc_blynk_setFirmwareInfo(CONFIG_BLYNK_FIRMWARE_TYPE,
                              CONFIG_BLYNK_FIRMWARE_VERSION,
                              BLYNK_FIRMWARE_BUILD_TIME,
                              BLYNK_RPC_LIB_VERSION);

#if defined(CONFIG_BLYNK_VENDOR_PREFIX)
    rpc_blynk_setVendorPrefix(CONFIG_BLYNK_VENDOR_PREFIX);
#endif
#if defined(CONFIG_BLYNK_VENDOR_SERVER)
    rpc_blynk_setVendorServer(CONFIG_BLYNK_VENDOR_SERVER);
#endif

#if defined(CONFIG_BLYNK_NCP_ONBOARD_RGB_LED)
    rpc_hw_initRGB(CONFIG_BLYNK_NCP_ONBOARD_RGB_R_GPIO_NUM,
                   CONFIG_BLYNK_NCP_ONBOARD_RGB_G_GPIO_NUM,
                   CONFIG_BLYNK_NCP_ONBOARD_RGB_B_GPIO_NUM,
                   CONFIG_BLYNK_NCP_ONBOARD_LED_COM_ANODE);
    rpc_hw_setLedBrightness(CONFIG_BLYNK_NCP_ONBOARD_LED_BRIGHT);
#endif

#if defined(CONFIG_BLYNK_NCP_ONBOARD_LED)
    rpc_hw_initLED(CONFIG_BLYNK_NCP_ONBOARD_LED_GPIO_NUM,
                   CONFIG_BLYNK_NCP_ONBOARD_LED_COM_ANODE);
    rpc_hw_setLedBrightness(CONFIG_BLYNK_NCP_ONBOARD_LED_BRIGHT);
#endif

#if defined(CONFIG_BLYNK_NCP_ONBOARD_BTN)
    rpc_hw_initUserButton(CONFIG_BLYNK_NCP_ONBOARD_BTN_GPIO_NUM,
                            CONFIG_BLYNK_NCP_ONBOARD_BTN_INVERSION);
#endif

    if (!rpc_blynk_initialize(CONFIG_BLYNK_TEMPLATE_ID, CONFIG_BLYNK_TEMPLATE_NAME))
    {
        LOG_ERR("rpc_blynk_initialize failed");
        return -1;
    }

    ncp_tid = k_thread_create(&ncp_thread_data, ncp_stack_area,
                                K_THREAD_STACK_SIZEOF(ncp_stack_area),
                                ncpThread,
                                NULL, NULL, NULL,
                                BLYNK_THREAD_PRIO, 0, K_NO_WAIT);

    k_timer_start(&ncpPingTimer, K_SECONDS(10), K_SECONDS(5));

    return 0;
}
