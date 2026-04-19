# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(modbus-rtu-project)

# Always include main.c
target_sources(app PRIVATE src/main.c)

# Conditionally include source files based on the selected role
target_sources_ifdef(CONFIG_MODBUS_ROLE_CLIENT app PRIVATE src/modbus_client.c)
target_sources_ifdef(CONFIG_MODBUS_ROLE_SERVER app PRIVATE src/modbus_server.c)
