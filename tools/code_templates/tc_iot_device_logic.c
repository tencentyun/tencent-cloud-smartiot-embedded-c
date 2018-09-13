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
/*${ data_template.local_data_initializer() }*/};

tc_iot_shadow_property_meta g_tc_iot_shadow_property_metas[] = {
/*${ data_template.property_def_initializer() }*/};

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
/*${data_template.generate_sample_code()}*/
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
            TC_IOT_LOG_TRACE("%dth id=%d, name=%s", i,g_tc_iot_shadow_property_metas[i].id, g_tc_iot_shadow_property_metas[i].name)
            return;
        }
    }
}

int _tc_iot_shadow_property_control_callback(tc_iot_event_message *msg, void * client) {
    int i = 0;
    tc_iot_mqtt_client * p_mqtt_client = client;
    tc_iot_shadow_client* c = TC_IOT_CONTAINER_OF(p_mqtt_client, tc_iot_shadow_client, mqtt_client);
    tc_iot_shadow_property_def fields[TC_IOT_PROPTOTAL];
    tc_iot_shadow_property_meta * p_metas = &g_tc_iot_shadow_property_metas[0];
    int count = 0;

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

    default:
        TC_IOT_LOG_TRACE("unkown event received, event=%d", msg->event);
        return TC_IOT_SUCCESS;
    }
    return TC_IOT_SUCCESS;
}

