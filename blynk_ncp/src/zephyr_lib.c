/*
 * Copyright (c) 2024 Blynk Technologies Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/linker/linker-defs.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(blynk_ncp, CONFIG_BLYNK_NCP_LOG_LEVEL);

#include <blynk_ncp/blynk_ncp.h>
#include <BlynkRpcClient.h>

BUILD_ASSERT(1 != sizeof(CONFIG_BLYNK_TEMPLATE_ID),         "BLYNK_TEMPLATE_ID is required");
BUILD_ASSERT(1 != sizeof(CONFIG_BLYNK_TEMPLATE_NAME),       "BLYNK_TEMPLATE_NAME is required");
BUILD_ASSERT(1 != sizeof(CONFIG_BLYNK_FIRMWARE_VERSION),    "BLYNK_FIRMWARE_VERSION is required");

#define BLYNK_FIRMWARE_BUILD_TIME __DATE__ " " __TIME__


#if DT_HAS_CHOSEN(blynk_ncp_status_led)
static const struct gpio_dt_spec staus_led = GPIO_DT_SPEC_GET(DT_CHOSEN(blynk_ncp_status_led), gpios);
#endif

#if DT_HAS_CHOSEN(blynk_ncp_rst)
static const struct gpio_dt_spec ncp_rst = GPIO_DT_SPEC_GET(DT_CHOSEN(blynk_ncp_rst), gpios);
#endif

K_THREAD_STACK_DEFINE(ncp_stack_area, CONFIG_BLYNK_THERAD_STACK_SIZE);
static struct k_thread ncp_thread_data;
static k_tid_t ncp_tid;

static uint8_t ncp_state = BLYNK_STATE_NOT_INITIALIZED;
static blynk_ncp_state_update_callback_t state_update_callback = NULL;
static blynk_ncp_event_callback_t event_callback = NULL;

#define BLYNK_PARAM_KV(k, v)    k "\0" v "\0"

K_MUTEX_DEFINE(ncp_rpc_mutex);

void ncp_uart_init(void);          // see zephyr_uart.c
int ncp_uart_set_br(uint32_t br);  // see zephyr_uart.c
void ncp_ota_run(void);            // see zephyr_ota.c

static void ncpPingHandler(struct k_work *work);

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
    const uint32_t br[] = { 38400, 115200, CONFIG_BLYNK_NCP_BAUD, 9600, 19200, 28800, 57600, 76800, 230400, 460800, 921600 };
    int i = 0;

    ncp_uart_init();
    RpcUartFraming_init();
    do
    {
        const int rc = ncp_uart_set_br(br[i]);
        if(rc)
        {
            LOG_ERR("can't set baudrate %d rc %d", br[i], rc);
            return false;
        }
        else
        {
            if (RPC_STATUS_OK != rpc_ncp_ping())
            {
                LOG_DBG("NCP not responding br %d", br[i]);
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
                LOG_DBG("setting target br %d", CONFIG_BLYNK_NCP_BAUD);

                rpc_hw_setUartBaudRate(CONFIG_BLYNK_NCP_BAUD);
                k_msleep(20);

                if(ncp_uart_set_br(CONFIG_BLYNK_NCP_BAUD))
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

    LOG_WRN("NCP not responding");

    return false;
}

K_WORK_DEFINE(ncpPingtWork, ncpPingHandler);

void ncpPingTimerHandler(struct k_timer *dummy)
{
    //timer called from system ISR so use work queue here to ping NCP later
    k_work_submit(&ncpPingtWork);
}

K_TIMER_DEFINE(ncpPingTimer, ncpPingTimerHandler, NULL);

static void ncpThread(void*, void*, void*)
{
    LOG_DBG("NCP init");
#if DT_HAS_CHOSEN(blynk_ncp_status_led)
    gpio_pin_configure_dt(&staus_led, GPIO_OUTPUT_INACTIVE);
#endif
#if DT_HAS_CHOSEN(blynk_ncp_rst)
    gpio_pin_configure_dt(&ncp_rst, GPIO_OUTPUT_INACTIVE);
    k_msleep(10);
#endif

    if (!ncpSetupSerial(5000))
    {
        LOG_ERR("can't setup serial");
        return;
    }

    const char* ncpFwVer = "unknown";
    if (!rpc_blynk_getNcpVersion(&ncpFwVer))
    {
        LOG_ERR("can't get NCP firmware version");
        return;
    }

    LOG_INF("NCP firmware: %s", ncpFwVer);

    rpc_blynk_setFirmwareInfo(CONFIG_BLYNK_FIRMWARE_TYPE,
                              CONFIG_BLYNK_FIRMWARE_VERSION,
                              BLYNK_FIRMWARE_BUILD_TIME,
                              BLYNK_RPC_LIB_VERSION);

    if (!rpc_blynk_initialize(CONFIG_BLYNK_TEMPLATE_ID, CONFIG_BLYNK_TEMPLATE_NAME))
    {
        LOG_ERR("rpc_blynk_initialize failed");
        return;
    }
    k_timer_start(&ncpPingTimer, K_SECONDS(10), K_SECONDS(CONFIG_BLYNK_NCP_PING_INTERVAL));

#if defined(CONFIG_BLYNK_VENDOR_PREFIX)
    if (1 != sizeof(CONFIG_BLYNK_VENDOR_PREFIX)) {
        rpc_blynk_setVendorPrefix(CONFIG_BLYNK_VENDOR_PREFIX);
    }
#endif
#if defined(CONFIG_BLYNK_VENDOR_SERVER)
    if (1 != sizeof(CONFIG_BLYNK_VENDOR_SERVER)) {
        rpc_blynk_setVendorServer(CONFIG_BLYNK_VENDOR_SERVER);
    }
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

    while(1)
    {
        rpc_run();

#if defined(CONFIG_BOOTLOADER_MCUBOOT)
        ncp_ota_run();
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
        if(err_num <= CONFIG_BLYNK_NCP_ERR_PING_CNT)
        {
            err_num++;
            LOG_INF("NCP ping error. attempt [%d]", err_num);
        }
        else
        {
            err_num = 0;

#if DT_HAS_CHOSEN(blynk_ncp_rst)
            LOG_WRN("NCP: hard reset");
            gpio_pin_configure_dt(&ncp_rst, GPIO_OUTPUT_ACTIVE);
            k_msleep(100);
            gpio_pin_configure_dt(&ncp_rst, GPIO_OUTPUT_INACTIVE);
            k_msleep(100);
#endif

#if ! DT_HAS_CHOSEN(blynk_ncp_rst)
            LOG_WRN("NCP: soft reboot");
            rpc_ncp_reboot();
            k_msleep(50);
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
    if(ncp_state == state)
    {
        // nothing changed
        return;
    }

    LOG_INF("NCP state: %s => %s", ncpGetStateString(ncp_state), ncpGetStateString(state));
    ncp_state = state;
    switch(state)
    {
        case BLYNK_STATE_NOT_INITIALIZED:
            // reinit in 1 second
            k_timer_start(&ncpReinitTimer, K_SECONDS(1), K_NO_WAIT);
            break;
        default:
            break;
    }

    if(state_update_callback)
    {
        state_update_callback(state);
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
        case RPC_EVENT_BLYNK_TIME_CHANGED:
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

    if(event_callback)
    {
        event_callback((RpcEvent)event);
    }
}

int blynk_ncp_init(void)
{
    static int init = 0;
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

    k_timer_stop(&ncpReinitTimer);
    k_timer_stop(&ncpPingTimer);
    if(init)
    {
        k_thread_abort(ncp_tid);
    }


    ncp_tid = k_thread_create(&ncp_thread_data, ncp_stack_area,
                              K_THREAD_STACK_SIZEOF(ncp_stack_area),
                              ncpThread,
                              NULL, NULL, NULL,
                              CONFIG_BLYNK_THREAD_PRIO, 0, K_NO_WAIT);

    init = 1;
    return 0;
}

void blynk_ncp_register_state_update_callback(blynk_ncp_state_update_callback_t cb)
{
    state_update_callback = cb;
}

void blynk_ncp_register_event_callback(blynk_ncp_event_callback_t cb)
{
    event_callback = cb;
}

RpcBlynkState blynk_ncp_get_state(void)
{
    return ncp_state;
}
