cmake_minimum_required(VERSION 3.20)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(centarus)

target_sources(app PRIVATE src/main.c)

# If we are building with server.conf, tell the C code
if(OVERLAY_CONFIG STREQUAL "server.conf")
    target_compile_definitions(app PRIVATE -DSERVER_ROLE=1)
elseif(OVERLAY_CONFIG STREQUAL "client.conf")
    target_compile_definitions(app PRIVATE -DCLIENT_ROLE=1)
endif()