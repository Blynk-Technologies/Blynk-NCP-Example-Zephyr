/*
 * Copyright (c) 2024 Blynk Technologies Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_BLYNK_RPC_CONFIG_H
#define ZEPHYR_BLYNK_RPC_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

void ncpMutexLock(void);
void ncpMutexUnlock(void);

#define RPC_MUTEX_LOCK()   ncpMutexLock()
#define RPC_MUTEX_UNLOCK() ncpMutexUnlock()

#if defined(RPC_INPUT_BUFFER)
/* OK, use it */
#elif (CONFIG_SRAM_SIZE <= 8)
#define RPC_INPUT_BUFFER 512
#endif

#if defined(RPC_ENABLE_SMALL_CRC8)
/* OK, use it */
#elif (CONFIG_FLASH_SIZE <= 32 || CONFIG_SRAM_SIZE <= 8)
#define RPC_ENABLE_SMALL_CRC8 1
#endif

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_BLYNK_RPC_CONFIG_H */
