
add_executable(test_http gtest.cpp tc_iot_http_test.cpp)
# Standard linking to gtest stuff.
target_link_libraries(test_http tc_iot_ota stdc++ pthread gtest gtest_main tc_iot_common )
add_test(NAME test_http COMMAND test_http)

add_executable(test_shadow gtest.cpp data_template/tc_iot_shadow_test.cpp)
target_link_libraries(test_shadow stdc++ pthread gtest gtest_main tc_iot_smart tc_iot_common tc_iot_token )
add_test(NAME test_shadow COMMAND test_shadow)

add_executable(test_mqtt gtest.cpp tc_iot_mqtt_test.cpp)
target_link_libraries(test_mqtt stdc++ pthread gtest gtest_main tc_iot_mqtt_client tc_iot_common tc_iot_token )
add_test(NAME test_mqtt COMMAND test_mqtt)

add_executable(test_smartiot gtest.cpp data_template/tc_iot_smart_test.cpp
    data_template/tc_iot_device_logic.cpp)
target_link_libraries(test_smartiot stdc++ pthread gtest gtest_main tc_iot_smart tc_iot_common tc_iot_token )
add_test(NAME test_smartiot COMMAND test_smartiot)


add_executable(test_hal gtest.cpp tc_iot_hal_test.cpp)
target_link_libraries(test_hal stdc++ pthread gtest gtest_main tc_iot_common tc_iot_hal)
add_test(NAME test_hal COMMAND test_hal)
