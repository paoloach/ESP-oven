cmake_minimum_required(VERSION 3.10)
project(ovenFirmware C CXX ASM)

set(NONOS_SDK /home/paolo/esp2866/espressif/ESP8266_NONOS_SDK-2.2.1)

set (core_src
        core/auth.c
        core/base64.c
        core/base64.h
        core/httpd.c
        core/httpd-nonos.c
        core/httpd-platform.h
        core/httpdespfs.c
        core/sha1.c)

set (espfs_src
        espfs/espfs.c
        espfs/espfsformat.h
        espfs/heatshrink_config_custom.h
        espfs/heatshrink_decoder.c)

set(mkupimg_src
        mkupgimg/mkupgimg.c)

set(util_src
        util/captdns.c
        util/cgiflash.c
        util/cgiwebsocket.c
        util/cgiwifi.c)

add_library(httpd STATIC ${core_src} ${espfs_src} ${mkupimg_src} ${util_src} )

target_compile_options(httpd  PRIVATE  -DHTTPD_STACKSIZE=2048 -DHTTPD_MAX_CONNECTIONS=4 -O0 -g3 -std=c99 -Werror -Wpointer-arith -Wundef -Wall -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals  -D__ets__ -DICACHE_FLASH -Wno-address)
target_include_directories(httpd PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${RTOS_DIR}/components/log/include
        ${CMAKE_SOURCE_DIR}/httpd/include
        ${CMAKE_SOURCE_DIR}/httpd
        ${CMAKE_SOURCE_DIR}/httpd/core
        ${CMAKE_SOURCE_DIR}/httpd/espfs
#        ${NONOS_SDK}/include
        )