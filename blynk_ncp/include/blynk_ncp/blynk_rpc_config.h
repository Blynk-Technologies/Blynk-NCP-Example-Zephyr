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

#define RPC_MUTEX_LOCK()    ncpMutexLock()
#define RPC_MUTEX_UNLOCK()  ncpMutexUnlock()

#ifdef __cplusplus
}
#endif

#endif /* ZEPHYR_BLYNK_RPC_CONFIG_H */

