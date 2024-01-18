#!/bin/env bash

# Copyright (c) 2024 Blynk Technologies Inc.
#
# SPDX-License-Identifier: Apache-2.0
set -e

clang-format -i ./blynk_ncp/src/*.c ./blynk_ncp/include/blynk_ncp/*.h ./samples/*/src/*.c
