#
# Copyright (c) 2024 Blynk Technologies Inc.
#
# SPDX-License-Identifier: Apache-2.0
#

PREFIX ?= .
OBJ_DIR ?= $(PREFIX)/obj
LIB_DIR ?= $(PREFIX)/lib

RPC_CONFIG = -include blynk_ncp/blynk_rpc_config.h

ifeq ($(findstring Windows, $(OS)),Windows)
	DEL := rmdir /S /Q
	MKDIR := mkdir
else
	DEL := rm -rf
	MKDIR := mkdir -p
endif

all:
	-$(MKDIR) "$(OBJ_DIR)"
	-$(MKDIR) "$(LIB_DIR)"
	$(CC) -MD -I../build/zephyr/include/generated/ -Iinclude -Isrc/BlynkNcpDriver/src $(CFLAGS) $(RPC_CONFIG) -c src/zephyr_uart.c -o $(OBJ_DIR)/zephyr_uart.o
	$(CC) -MD -I../build/zephyr/include/generated/ -Iinclude -Isrc/BlynkNcpDriver/src $(CFLAGS) $(RPC_CONFIG) -c src/zephyr_ota.c -o $(OBJ_DIR)/zephyr_ota.o
	$(CC) -MD -I../build/zephyr/include/generated/ -Iinclude -Isrc/BlynkNcpDriver/src $(CFLAGS) $(RPC_CONFIG) -c src/zephyr_lib.c -o $(OBJ_DIR)/zephyr_lib.o
	$(CC) -MD -I../build/zephyr/include/generated/ -Iinclude -Isrc/BlynkNcpDriver/src $(CFLAGS) $(RPC_CONFIG) -c src/BlynkNcpDriver/src/BlynkRpcCRC8.c -o $(OBJ_DIR)/BlynkRpcCRC8.o
	$(CC) -MD -I../build/zephyr/include/generated/ -Iinclude -Isrc/BlynkNcpDriver/src $(CFLAGS) $(RPC_CONFIG) -c src/BlynkNcpDriver/src/BlynkRpcClientWeakImpl.c -o $(OBJ_DIR)/BlynkRpcClientWeakImpl.o
	$(CC) -MD -I../build/zephyr/include/generated/ -Iinclude -Isrc/BlynkNcpDriver/src $(CFLAGS) $(RPC_CONFIG) -c src/BlynkNcpDriver/src/MessageWriter.c -o $(OBJ_DIR)/MessageWriter.o
	$(CC) -MD -I../build/zephyr/include/generated/ -Iinclude -Isrc/BlynkNcpDriver/src $(CFLAGS) $(RPC_CONFIG) -c src/BlynkNcpDriver/src/BlynkRpcInfraUart.c -o $(OBJ_DIR)/BlynkRpcInfraUart.o
	$(CC) -MD -I../build/zephyr/include/generated/ -Iinclude -Isrc/BlynkNcpDriver/src $(CFLAGS) $(RPC_CONFIG) -c src/BlynkNcpDriver/src/MessageBuffer.c -o $(OBJ_DIR)/MessageBuffer.o
	$(CC) -MD -I../build/zephyr/include/generated/ -Iinclude -Isrc/BlynkNcpDriver/src $(CFLAGS) $(RPC_CONFIG) -c src/BlynkNcpDriver/src/BlynkRpcUartFraming.c -o $(OBJ_DIR)/BlynkRpcUartFraming.o
	$(CC) -MD -I../build/zephyr/include/generated/ -Iinclude -Isrc/BlynkNcpDriver/src $(CFLAGS) $(RPC_CONFIG) -c src/BlynkNcpDriver/src/BlynkRpcInfra.c -o $(OBJ_DIR)/BlynkRpcInfra.o
	$(AR) -rcs $(LIB_DIR)/libblynkncp.a $(OBJ_DIR)/*.o

clean:
	$(DEL) "$(OBJ_DIR)" "$(LIB_DIR)"
