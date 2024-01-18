/*
 * Copyright (c) 2024 Blynk Technologies Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#if defined(CONFIG_BOOTLOADER_MCUBOOT)

#include <stdio.h>
#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/sys/reboot.h>

#include <blynk_ncp/blynk_ncp.h>

#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(blynk_ncp);

typedef enum BlynkOtaState {
    OTA_STATE_IDLE,
    OTA_STATE_PREFETCH,
    OTA_STATE_START,
    OTA_STATE_IN_PROGRESS,
    OTA_STATE_APPLY,

    OTA_STATE_QTY
} BlynkOtaState;

static struct {
    uint32_t size;
    uint32_t offset;
    uint32_t crc32;
    int progress;
    BlynkOtaState state;
    const struct flash_area *fa;
} ota_info;

// #define BLYNK_NCP_OTA_PREFETCH
// #warning BLYNK_NCP_OTA_PREFETCH hardcoded

static uint32_t calcCRC32(const void *data, size_t length, uint32_t previousCrc32)
{
    // NOTE: normal representation for Polynomial is 04C11DB7
    const uint32_t Polynomial = 0xEDB88320; // reversed representation
    uint32_t crc = ~previousCrc32;
    const uint8_t *ptr = (const uint8_t *)data;
    while (length--) {
        crc ^= *ptr++;
        for (unsigned int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ (-(crc & 1) & Polynomial);
        }
    }
    return ~crc;
}

bool rpc_client_otaUpdateAvailable_impl(const char *filename, uint32_t filesize,
                                        const char *fw_type, const char *fw_ver,
                                        const char *fw_build)
{
    const uint32_t maxSize = FIXED_PARTITION_SIZE(slot1_partition);
    if (!maxSize) {
        LOG_ERR("OTA is not supported");
        return false;
    }

    uint32_t percentSize = ((filesize * 100) / maxSize);
    LOG_INF("OTA update: %s size: %d (%d%%), type: %s, version: %s, build: %s", filename, filesize,
            percentSize, fw_type, fw_ver, fw_build);

    if (filesize == 0 || filesize > maxSize) {
        LOG_ERR("File size is invalid");
        return false;
    }

    // Prepare for OTA update
    if (0 == flash_area_erase(ota_info.fa, 0, FIXED_PARTITION_SIZE(slot1_partition))) {
        LOG_INF("Starting OTA");
        ota_info.size = filesize;
        ota_info.offset = 0;
        ota_info.crc32 = 0;
        ota_info.progress = 0;

#if defined(BLYNK_NCP_OTA_PREFETCH)
        ota_info.state = OTA_STATE_PREFETCH;
#else
        ota_info.state = OTA_STATE_START;
#endif
        return true;
    } else {
        LOG_ERR("Starting OTA failed");
    }

    return false;
}

bool rpc_client_otaUpdateWrite_impl(uint32_t offset, rpc_buffer_t chunk, uint32_t crc32)
{
    bool crcOK = (calcCRC32(chunk.data, chunk.length, 0) == crc32);

    LOG_DBG("OTA chunk @%06x, size: %u, crc: %08x => %s", offset, chunk.length, crc32,
            crcOK ? "OK" : "fail!");

    if (!crcOK) {
        return false;
    }
    if (ota_info.offset != offset) {
        LOG_ERR("Offset mismatch");
        return false;
    }

    // Compute cumulative CRC32
    ota_info.crc32 = calcCRC32(chunk.data, chunk.length, ota_info.crc32);

    // Store the update
    const size_t al = flash_area_align(ota_info.fa);
    const uint8_t *data = chunk.data;

    const size_t before = ota_info.offset % al;
    if (before) {
        const size_t o = (ota_info.offset / al) * al;
        const size_t c = al - before;

        uint8_t *buf = k_malloc(al);
        if (buf) {
            if (flash_area_read(ota_info.fa, o, buf, al)) {
                LOG_ERR("can't read data offset[%d] len[%d]", o, al);
                k_free(buf);
                return false;
            }

            memcpy(buf + before, data, c);
            if (flash_area_write(ota_info.fa, o, buf, al)) {
                LOG_ERR("can't write data offset[%d] len[%d]", o, al);
                k_free(buf);
                return false;
            }
            k_free(buf);

            data += c;
            chunk.length -= c;
            ota_info.offset += c;
        } else {
            LOG_ERR("can't allocate %d bytes", al);
            return false;
        }
    }

    const size_t body = al * (chunk.length / al);
    if (body) {
        if (flash_area_write(ota_info.fa, ota_info.offset, data, body)) {
            LOG_ERR("can't write data offset[%d] len[%d]", ota_info.offset, body);
            return false;
        }
        data += body;
        chunk.length -= body;
        ota_info.offset += body;
    }

    const size_t tail = chunk.length;
    if (tail) {
        uint8_t *buf = k_malloc(al);
        if (buf) {
            memcpy(buf, data, tail);
            memset(buf + tail, flash_area_erased_val(ota_info.fa), al - tail);
            if (flash_area_write(ota_info.fa, ota_info.offset, buf, al)) {
                LOG_ERR("can't write data offset[%d] len[%d]", ota_info.offset, al);
                k_free(buf);
                return false;
            }
            k_free(buf);
            data += tail;
            chunk.length -= tail;
            ota_info.offset += tail;
        } else {
            LOG_ERR("can't allocate %d bytes", al);
            return false;
        }
    }

    const int progress = (ota_info.offset * 100) / ota_info.size;
    if (progress - ota_info.progress >= 5 || progress == 100) {
        ota_info.progress = progress;
        LOG_DBG("Updating MCU... %d%%", progress);
    }

    return true;
}

bool rpc_client_otaUpdateFinish_impl(void)
{
    if (ota_info.offset != ota_info.size) {
        LOG_ERR("File size mismatch");
        return false;
    }

    rpc_buffer_t digest;
    if (rpc_blynk_otaUpdateGetMD5(&digest)) {
        // LOG_HEX("Expected MD5:    ", digest.data, digest.length);
    }
    if (rpc_blynk_otaUpdateGetSHA256(&digest)) {
        // LOG_HEX("Expected SHA256: ", digest.data, digest.length);
    }

    uint32_t expectedCRC32 = 0;
    if (!rpc_blynk_otaUpdateGetCRC32(&expectedCRC32)) {
        LOG_ERR("Cannot get CRC32");
        return false;
    }
    if (expectedCRC32 != ota_info.crc32) {
        LOG_ERR("CRC32 check failed (expected: %08x, actual: %08x)", expectedCRC32, ota_info.crc32);
        return false;
    }

    LOG_DBG("CRC32 verified: %08x", ota_info.crc32);

    // Apply the update and restart
    flash_area_close(ota_info.fa);
    ota_info.state = OTA_STATE_APPLY;

    return true;
}

void rpc_client_otaUpdateCancel_impl(void)
{
    LOG_ERR("OTA canceled");
    flash_area_close(ota_info.fa);
}

void ncp_ota_run(void)
{
    static int init = 0;

    if (!init) {
        if (0 == flash_area_open(FIXED_PARTITION_ID(slot1_partition), &ota_info.fa)) {
            init = 1;
        }
    }

    switch (ota_info.state) {
#if defined(BLYNK_NCP_OTA_PREFETCH)
    case OTA_STATE_PREFETCH: {
        uint8_t prefetch = rpc_blynk_otaUpdatePrefetch();
        if (RPC_OTA_PREFETCH_OK == prefetch) {
            LOG_INF("OTA prefetch OK");
        } else {
            LOG_WRN("OTA prefetch FAILED");
        }
        ota_info.state = OTA_STATE_START;
    } break;
#endif
    case OTA_STATE_START: {
        const size_t al = flash_area_align(ota_info.fa);
        const uint16_t chunk_size = (uint16_t)(1024 < al ? 1024 : (1024 / al * al));
        rpc_blynk_otaUpdateStart(chunk_size);
        ota_info.state = OTA_STATE_IN_PROGRESS;
    } break;
    case OTA_STATE_APPLY: {
        ota_info.state = OTA_STATE_IDLE;
        LOG_INF("Applying the update");
        if (boot_set_pending_multi(0, 0)) {
            LOG_ERR("ERROR during Applying the update");
        } else {
            k_msleep(100);
            // Apply the update and restart
            sys_reboot(SYS_REBOOT_COLD);
        }
    } break;
    default:
        break;
    }
}

#endif /* CONFIG_BOOTLOADER_MCUBOOT */
