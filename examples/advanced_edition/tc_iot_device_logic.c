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
    false,
    TC_IOT_PROP_param_enum_enum_a,
    0,
    {'\0'},
};

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
        0,  /* send will */
        {
            {'M', 'Q', 'T', 'W'}, 0, {NULL, {0, NULL}}, {NULL, {0, NULL}}, 0, 0,
        }
    },
    TC_IOT_SHADOW_SUB_TOPIC_DEF,
    TC_IOT_SHADOW_PUB_TOPIC_DEF,
    tc_iot_device_on_message_received,
    _tc_iot_shadow_property_control_callback,
};


static int _tc_iot_property_change( const char * name, const char * value) {
    tc_iot_shadow_bool param_bool;
    tc_iot_shadow_enum param_enum;
    tc_iot_shadow_number param_number;
    tc_iot_shadow_string param_string;
    if (strcmp("param_bool", name) == 0) {
        param_bool = atoi(value);
        g_tc_iot_device_local_data.param_bool = param_bool;
        if (param_bool) {
            TC_IOT_LOG_TRACE("do something for param_bool on");
        } else {
            TC_IOT_LOG_TRACE("do something for param_bool off");
        }
        goto operate;
    }
    if (strcmp("param_enum", name) == 0) {
        param_enum = atoi(value);
        g_tc_iot_device_local_data.param_enum = param_enum;
        switch(param_enum){
            case TC_IOT_PROP_param_enum_enum_a:
                TC_IOT_LOG_TRACE("do something for param_enum = enum_a");
                break;
            case TC_IOT_PROP_param_enum_enum_b:
                TC_IOT_LOG_TRACE("do something for param_enum = enum_b");
                break;
            case TC_IOT_PROP_param_enum_enum_c:
                TC_IOT_LOG_TRACE("do something for param_enum = enum_c");
                break;
            default:
                TC_IOT_LOG_WARN("do something for param_enum = unknown");
                return TC_IOT_FAILURE;
        }
        goto operate;
    }
    if (strcmp("param_number", name) == 0) {
        param_number = atof(value);
        g_tc_iot_device_local_data.param_number = param_number;
        TC_IOT_LOG_TRACE("do something for param_number=%f", param_number);
        goto operate;
    }
    if (strcmp("param_string", name) == 0) {
        param_string = (char *)value;
        strcpy(g_tc_iot_device_local_data.param_string, param_string);
        TC_IOT_LOG_TRACE("do something for param_string=%s", param_string);
        goto operate;
    }
    TC_IOT_LOG_WARN("unkown %s = %s", name, value);
    return TC_IOT_FAILURE;
    operate:
    TC_IOT_LOG_TRACE("operating device");
    operate_device(&g_tc_iot_device_local_data);
    return TC_IOT_SUCCESS;

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

