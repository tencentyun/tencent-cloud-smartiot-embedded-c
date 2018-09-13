
extern "C" {
#include "tc_iot_inc.h"
#include "tc_iot_device_config.h"
}
#include "gtest/gtest.h"

TEST(tc_iot_hal_testcases, tc_iot_hal_get_config)
{
    int ret = 0;
    const char * res;
    char buffer[1024];

    res = tc_iot_hal_get_config(TC_IOT_DCFG_PRODUCT_ID, buffer, sizeof(buffer), "");

    ret = tc_iot_hal_set_config(TC_IOT_DCFG_PRODUCT_ID, TC_IOT_CONFIG_PRODUCT_ID);
    ASSERT_EQ(ret, TC_IOT_SUCCESS);
    ret = tc_iot_hal_set_config(TC_IOT_DCFG_PRODUCT_KEY, TC_IOT_CONFIG_PRODUCT_KEY);
    ASSERT_EQ(ret, TC_IOT_SUCCESS);
    ret = tc_iot_hal_set_config(TC_IOT_DCFG_DEVICE_NAME, TC_IOT_CONFIG_DEVICE_NAME);
    ASSERT_EQ(ret, TC_IOT_SUCCESS);
    ret = tc_iot_hal_set_config(TC_IOT_DCFG_DEVICE_SECRET, TC_IOT_CONFIG_DEVICE_SECRET);
    ASSERT_EQ(ret, TC_IOT_SUCCESS);
    ret = tc_iot_hal_set_config(TC_IOT_DCFG_MQTT_HOST, TC_IOT_CONFIG_MQTT_HOST);
    ASSERT_EQ(ret, TC_IOT_SUCCESS);
    ret = tc_iot_hal_set_config(TC_IOT_DCFG_API_HOST, TC_IOT_CONFIG_API_HOST);
    ASSERT_EQ(ret, TC_IOT_SUCCESS);

    res = tc_iot_hal_get_config(TC_IOT_DCFG_PRODUCT_ID, buffer, sizeof(buffer), "");
    ASSERT_EQ(res, buffer);
    ASSERT_STREQ(buffer, TC_IOT_CONFIG_PRODUCT_ID);
}

