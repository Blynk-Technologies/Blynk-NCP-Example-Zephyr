/*
 * Copyright (c) 2024 Blynk Technologies Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef BLYNK_LIB_H
#define BLYNK_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "BlynkRpc.h"
#include "idl/rpc_shim_ncp.h"
#include "idl/rpc_shim_hw.h"
#include "idl/rpc_shim_blynk.h"
#include "BlynkRpcUartFraming.h"

int blynk_ncp_init(void);


#if defined(CONFIG_MCUBOOT_BOOTUTIL_LIB)
/* TODO: resolve external library helper functions */
extern int boot_set_confirmed_multi(int image_index);
extern int boot_set_pending_multi(int image_index, int permanent);
#endif

#ifdef __cplusplus
}
#endif

#endif
