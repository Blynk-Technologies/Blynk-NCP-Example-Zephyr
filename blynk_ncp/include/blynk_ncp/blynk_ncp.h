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

typedef void (*blynk_ncp_state_update_callback_t)(RpcBlynkState);
void blynk_ncp_register_state_update_callback(blynk_ncp_state_update_callback_t cb);


typedef void (*blynk_ncp_event_callback_t)(RpcEvent);
void blynk_ncp_register_event_callback(blynk_ncp_event_callback_t cb);

RpcBlynkState blynk_ncp_get_state(void);

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
