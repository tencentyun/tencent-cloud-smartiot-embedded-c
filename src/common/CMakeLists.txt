list(APPEND TC_IOT_COMMON_SRCS tc_iot_base64.c tc_iot_log.c tc_iot_http_utils.c tc_iot_http_client.c
    tc_iot_string_utils.c tc_iot_sign_utils.c tc_iot_endian.c)

if (ENABLE_MQTT)
    list(APPEND TC_IOT_COMMON_SRCS tc_iot_hmac.c tc_iot_json.c tc_iot_json_writer.c tc_iot_md5.c tc_iot_url.c tc_iot_ya_buffer.c)
endif()

if (ENABLE_TLS)
    list(APPEND TC_IOT_COMMON_SRCS tc_iot_certs.c)
endif()

if (ENABLE_SNTP_CLIENT)
    list(APPEND TC_IOT_COMMON_SRCS tc_iot_sntp.c)
endif()

add_library(tc_iot_common STATIC ${TC_IOT_COMMON_SRCS})

target_link_libraries(tc_iot_common jsmn)

