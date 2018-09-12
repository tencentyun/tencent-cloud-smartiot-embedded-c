#include "tc_iot_device_config.h"
#include "tc_iot_device_logic.h"
#include "tc_iot_export.h"

int _tc_iot_shadow_property_control_callback(tc_iot_event_message *msg, void * client);
void operate_device(tc_iot_shadow_local_data * device);

/* 影子数据 Client  */
tc_iot_shadow_client g_tc_iot_shadow_client;

tc_iot_shadow_client * tc_iot_get_shadow_client(void) {
    return &g_tc_iot_shadow_client;
}


/* 设备当前状态数据 */
tc_iot_shadow_local_data g_tc_iot_device_local_data = {
/*${ data_template.local_data_initializer() }*/};

/* 设备初始配置 */
tc_iot_shadow_config g_tc_iot_shadow_config = {
    {
        {
            /* device info*/
            TC_IOT_CONFIG_DEVICE_SECRET, TC_IOT_CONFIG_PRODUCT_ID, TC_IOT_CONFIG_PRODUCT_KEY,
            TC_IOT_CONFIG_DEVICE_NAME, TC_IOT_CONFIG_DEVICE_CLIENT_ID,
            TC_IOT_CONFIG_DEVICE_USER_NAME, TC_IOT_CONFIG_DEVICE_PASSWORD, 0,
            TC_IOT_CONFIG_AUTH_MODE, TC_IOT_CONFIG_REGION, TC_IOT_CONFIG_API_HOST,
            TC_IOT_CONFIG_MQTT_HOST,
            TC_IOT_CONFIG_MQTT_PORT,
        },
        TC_IOT_CONFIG_USE_TLS,
        NULL,
        NULL,
    },
    TC_IOT_SHADOW_SUB_TOPIC_DEF,
    TC_IOT_SHADOW_PUB_TOPIC_DEF,
    tc_iot_device_on_message_received,
    _tc_iot_shadow_property_control_callback,
};


static int _tc_iot_property_change( const char * name, const char * value) {
/*${data_template.generate_sample_code()}*/
}

int _tc_iot_shadow_property_control_callback(tc_iot_event_message *msg, void * client) {

    if (!msg) {
        TC_IOT_LOG_ERROR("msg is null.");
        return TC_IOT_FAILURE;
    }

    if (msg->event == TC_IOT_SHADOW_EVENT_SERVER_CONTROL) {
        return _tc_iot_property_change((const char *)msg->context, (const char *)msg->data);
    } else {
        TC_IOT_LOG_TRACE("unkown event received, event=%d", msg->event);
    }
    return TC_IOT_SUCCESS;
}

