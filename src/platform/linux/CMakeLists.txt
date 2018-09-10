
list(APPEND tc_iot_hal_srcs tc_iot_hal_util.c tc_iot_hal_os.c tc_iot_hal_timer.c)

if (ENABLE_MQTT)
    if(ENABLE_TLS)
        list(APPEND tc_iot_hal_srcs tc_iot_hal_net.c tc_iot_hal_tls.c)
    else()
        list(APPEND tc_iot_hal_srcs tc_iot_hal_net.c)
    endif()
endif()


add_library(tc_iot_hal STATIC ${tc_iot_hal_srcs})

if(ENABLE_TLS)
    target_link_libraries(tc_iot_hal mbedtls)
endif()


