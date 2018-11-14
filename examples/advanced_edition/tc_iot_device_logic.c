#include "tc_iot_device_config.h"
#include "tc_iot_device_logic.h"
#include "tc_iot_export.h"

int _tc_iot_shadow_property_control_callback(tc_iot_event_message *msg, void * client);
void operate_device(unsigned char * changed_bits, tc_iot_shadow_local_data * device);

/* 影子数据 Client  */
tc_iot_shadow_client g_tc_iot_shadow_client;

tc_iot_shadow_client * tc_iot_get_shadow_client(void) {
    return &g_tc_iot_shadow_client;
}


/* 设备当前状态数据 */
tc_iot_shadow_local_data g_tc_iot_device_local_data = {
    false,
    TC_IOT_PROP_E_param_enum_0,
    0,
    {'\0'},
};

tc_iot_shadow_property_meta g_tc_iot_shadow_property_metas[] = {
    { "param_bool", TC_IOT_SHADOW_TYPE_BOOL, TC_IOT_PROP_param_bool, &g_tc_iot_device_local_data.param_bool},
    { "param_enum", TC_IOT_SHADOW_TYPE_ENUM, TC_IOT_PROP_param_enum, &g_tc_iot_device_local_data.param_enum},
    { "param_number", TC_IOT_SHADOW_TYPE_NUMBER, TC_IOT_PROP_param_number, &g_tc_iot_device_local_data.param_number},
    { "param_string", TC_IOT_SHADOW_TYPE_STRING, TC_IOT_PROP_param_string, &g_tc_iot_device_local_data.param_string},
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
        _tc_iot_shadow_property_control_callback,
        tc_iot_device_on_message_received,
    },
    TC_IOT_SHADOW_SUB_TOPIC_DEF,
    TC_IOT_SHADOW_PUB_TOPIC_DEF,
    TC_IOT_PROPTOTAL,
    &g_tc_iot_shadow_property_metas[0],
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
         return TC_IOT_SUCCESS;
    }
    if (strcmp("param_enum", name) == 0) {
        param_enum = atoi(value);
        g_tc_iot_device_local_data.param_enum = param_enum;
        switch(param_enum){
            case TC_IOT_PROP_E_param_enum_0:
                TC_IOT_LOG_TRACE("do something for param_enum = enum_a");
                break;
            case TC_IOT_PROP_E_param_enum_1:
                TC_IOT_LOG_TRACE("do something for param_enum = enum_b");
                break;
            case TC_IOT_PROP_E_param_enum_2:
                TC_IOT_LOG_TRACE("do something for param_enum = enum_c");
                break;
            default:
                TC_IOT_LOG_WARN("do something for param_enum = unknown");
                return TC_IOT_FAILURE;
        }
         return TC_IOT_SUCCESS;
    }
    if (strcmp("param_number", name) == 0) {
        param_number = atof(value);
        g_tc_iot_device_local_data.param_number = param_number;
        TC_IOT_LOG_TRACE("do something for param_number=%f", param_number);
         return TC_IOT_SUCCESS;
    }
    if (strcmp("param_string", name) == 0) {
        param_string = (char *)value;
        strcpy(g_tc_iot_device_local_data.param_string, param_string);
        TC_IOT_LOG_TRACE("do something for param_string=%s", param_string);
         return TC_IOT_SUCCESS;
    }
    TC_IOT_LOG_WARN("unkown %s = %s", name, value);
    return TC_IOT_FAILURE;

}


static unsigned char _g_tc_iot_changed_bits[(TC_IOT_PROPTOTAL-1)/8 + 1];

static void _tc_iot_clear_changed_bit() {
    memset(_g_tc_iot_changed_bits, 0, sizeof(_g_tc_iot_changed_bits));
}

static void _tc_iot_set_changed_bit(const char * name) {
    int i = 0;
    if (!name) {
        TC_IOT_LOG_ERROR("name is null");
        return ;
    }

    for ( i = 0; i < TC_IOT_PROPTOTAL; i++) {
        if (strcmp(name, g_tc_iot_shadow_property_metas[i].name) == 0) {
            TC_IOT_BIT_SET(_g_tc_iot_changed_bits, g_tc_iot_shadow_property_metas[i].id);
            TC_IOT_LOG_TRACE("%dth id=%d, name=%s", i,
                             g_tc_iot_shadow_property_metas[i].id,
                             g_tc_iot_shadow_property_metas[i].name);
            return;
        }
    }
}

int _tc_iot_shadow_property_control_callback(tc_iot_event_message *msg, void * client) {
    int i = 0;
    tc_iot_mqtt_client * p_mqtt_client = (tc_iot_mqtt_client *)client;
    tc_iot_shadow_client* c = TC_IOT_CONTAINER_OF(p_mqtt_client, tc_iot_shadow_client, mqtt_client);
    tc_iot_shadow_property_def fields[TC_IOT_PROPTOTAL];
    tc_iot_shadow_property_meta * p_metas = &g_tc_iot_shadow_property_metas[0];
    int count = 0;
    const char * name = NULL;
    const char * value = NULL;
    int log_level = 0;

    if (!msg) {
        TC_IOT_LOG_ERROR("msg is null.");
        return TC_IOT_FAILURE;
    }

    switch (msg->event) {
    case TC_IOT_SHADOW_EVENT_BEFORE_SERVER_CONTROL:
        TC_IOT_LOG_TRACE("before server control, event=%d", msg->event);
        break;
    case TC_IOT_SHADOW_EVENT_SERVER_CONTROL:
        _tc_iot_set_changed_bit((const char *)msg->context);
        return _tc_iot_property_change((const char *)msg->context, (const char *)msg->data);
    case TC_IOT_SHADOW_EVENT_AFTER_SERVER_CONTROL:
        TC_IOT_LOG_TRACE("after server control, event=%d", msg->event);

        for (i = 0; i < TC_IOT_PROPTOTAL; i++) {
            if (TC_IOT_BIT_GET(_g_tc_iot_changed_bits, i)) {
                fields[count].name = p_metas[i].name;
                fields[count].type = p_metas[i].type;
                fields[count].value = p_metas[i].value;
                TC_IOT_LOG_TRACE("%s type=%d", fields[count].name, fields[count].type);
                count ++;
            }
        }

        if (count) {
            operate_device(_g_tc_iot_changed_bits, &g_tc_iot_device_local_data);
            tc_iot_report_device_data(c, count, fields);
            tc_iot_confirm_device_data(c, count, fields);
            _tc_iot_clear_changed_bit();
        }

        break;
    case TC_IOT_SHADOW_EVENT_REMOTE_CONF:
        name = msg->context;
        value = msg->data;
        TC_IOT_LOG_TRACE("remote conf: %s=%s", name, value);
        if (strcmp(name,"reboot") == 0) {
            if (value[0] == '1') {
                TC_IOT_LOG_WARN("Reboot command received, please restart this program manually.");
            } else {
                TC_IOT_LOG_ERROR("remote_conf/reboot has invalid value=%s", value);
            }
        } else if (strcmp(name,"log_level") == 0) {
            log_level = atoi(value);
            TC_IOT_LOG_INFO("remote_conf/log_level set new log level=%s", value);
            // 0，默认不打； 1. 简要 2：详细
            if (log_level == 0) {
                tc_iot_set_log_level(TC_IOT_LOG_LEVEL_ERROR);
            } else if (log_level == 1) {
                tc_iot_set_log_level(TC_IOT_LOG_LEVEL_INFO);
            } else if (log_level == 2) {
                tc_iot_set_log_level(TC_IOT_LOG_LEVEL_TRACE);
            } else {
                TC_IOT_LOG_INFO("remote_conf/log_level has invalid value=%s", value);
                return TC_IOT_SUCCESS;
            }
            tc_iot_hal_set_config(TC_IOT_DCFG_LOG_LEVEL, value);
        } else if (strcmp(name,"is_up_busilog") == 0) {
            TC_IOT_LOG_TRACE("remote_conf/is_up_busilog=%s", value);
            if (value[0] == '1') {
                tc_iot_set_is_up_busilog(1);
            } else if (value[0] == '0') {
                tc_iot_set_is_up_busilog(0);
            } else {
                TC_IOT_LOG_ERROR("remote_conf/is_up_busilog has invalid value=%s", value);
                return TC_IOT_SUCCESS;
            }
            tc_iot_hal_set_config(TC_IOT_DCFG_IS_UP_BUSILOG, value);
        } else {
            TC_IOT_LOG_ERROR("unknown remote_conf parameter found:%s=%s",  name, value);
        }
        break;

    default:
        TC_IOT_LOG_TRACE("event=%d ignored.", msg->event);
        return TC_IOT_SUCCESS;
    }
    return TC_IOT_SUCCESS;
}

