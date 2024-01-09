/*
 * Copyright (c) 2024 Blynk Technologies Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/posix/unistd.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>

#include <zephyr/logging/log.h>
#include <zephyr/sys/ring_buffer.h>

#include "BlynkRpc.h"

LOG_MODULE_DECLARE(blynk_ncp);

#ifndef CONFIG_BLYNK_UART_RX_BUF_SIZE
#error CONFIG_BLYNK_UART_RX_BUF_SIZE not defined
#endif

#ifndef CONFIG_BLYNK_UART_TX_BUF_SIZE
#error CONFIG_BLYNK_UART_TX_BUF_SIZE not defined
#endif

static const struct device *const uart_dev = DEVICE_DT_GET(DT_CHOSEN(blynk_ncp_uart));

RING_BUF_DECLARE(rx_buf, CONFIG_BLYNK_UART_RX_BUF_SIZE);
RING_BUF_DECLARE(tx_buf, CONFIG_BLYNK_UART_TX_BUF_SIZE);

static bool tx_buf_is_empty(void)
{
    return ring_buf_size_get(&tx_buf) == 0;
}

static void serial_cb(const struct device *dev, void *user_data)
{
    if (!uart_irq_update(uart_dev))
    {
        return;
    }

    while (uart_irq_rx_ready(uart_dev))
    {
        uint8_t rx_data;
        while(uart_fifo_read(uart_dev, &rx_data, 1) == 1)
        {
            if (ring_buf_space_get(&rx_buf) == 0)
            {
                LOG_ERR("UART RX ring buffer is full");
                return;
            }
            ring_buf_put(&rx_buf, &rx_data, 1);
        }
    }

    while (uart_irq_tx_ready(uart_dev))
    {
        uint8_t tx_data;
        const uint32_t size = ring_buf_get(&tx_buf, &tx_data, 1);

        if (size == 1)
            uart_fifo_fill(uart_dev, &tx_data, 1);
        else
            uart_irq_tx_disable(uart_dev);
    }
}

int rpc_uart_available(void)
{
    return !!ring_buf_size_get(&rx_buf);
}

int rpc_uart_read(void)
{
    uint8_t tmp;
    ring_buf_get(&rx_buf, &tmp, 1);
    return tmp;
}

size_t rpc_uart_write(uint8_t data)
{
    if (ring_buf_space_get(&tx_buf) == 0)
    {
        LOG_ERR("UART TX ring buffer is full");
        return 0;
    }

    const int len = ring_buf_put(&tx_buf, &data, 1);
    if (len > 0)
        uart_irq_tx_enable(uart_dev);

    return len;
}

void rpc_uart_flush(void)
{
    while (!tx_buf_is_empty())
    {
        uart_irq_tx_enable(uart_dev);
        k_sleep(K_MSEC(1));
    }
}

uint32_t rpc_system_millis(void)
{
    return k_uptime_get_32();
}

int ncp_uart_init(void)
{
    if (!device_is_ready(uart_dev))
    {
        LOG_WRN("UART device not found!");
        return 0;
    }

    const int ret = uart_irq_callback_user_data_set(uart_dev, serial_cb, NULL);

    if (ret)
    {
        if (ret == -ENOTSUP)
        {
            LOG_WRN("Interrupt-driven UART API support not enabled");
        }
        else if (ret == -ENOSYS)
        {
            LOG_WRN("UART device does not support interrupt-driven API");
        }
        else
        {
            LOG_WRN("Error setting UART callback: %d", ret);
        }
    }
    else
    {
        uart_irq_rx_enable(uart_dev);
    }
    return ret;
}

int uart_set_br(uint32_t br)
{
    static struct uart_config uart_cfg = {
        .parity = UART_CFG_PARITY_NONE,
        .stop_bits = UART_CFG_STOP_BITS_1,
        .flow_ctrl = UART_CFG_FLOW_CTRL_NONE,
        .data_bits = UART_CFG_DATA_BITS_8,
    };

    uart_cfg.baudrate = br;
    return uart_configure(uart_dev, &uart_cfg);
}

