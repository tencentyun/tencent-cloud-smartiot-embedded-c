#ifndef TC_IOT_EVENT_H
#define TC_IOT_EVENT_H

#define EVENT_OBJ_DEF_BEGIN(def_name)   tc_iot_shadow_property_def  def_name []  = {
#define EVENT_OBJ_FIELD_DEF(value, TYPE , field)    { #field , TYPE, &(value.field) },
#define EVENT_OBJ_DEF_END     } ;

#define EVENT_LIST_DEF_BEGIN(def_name)   tc_iot_shadow_property_def  def_name []  = {
#define EVENT_SINGLE_DEF( TYPE , field)    { #field , TYPE, &(field) },
#define EVENT_LIST_DEF_END     } ;

#define EVENT_NUM_DEF(__local_event_def)  (sizeof(__local_event_def)/sizeof(__local_event_def[0]))


#define REPORT_SINGLE_ERROR(  client , error_type, error_value ) \
tc_iot_report_single_error(client, #error_value , error_type, &(error_value))

#define TC_IOT_REPORT_EVENT_ERROR_MSG_LEN TC_IOT_REPORT_UPDATE_MSG_LEN


#define IF_FALSE_RETURN(a, ret)                                            \
    do {                                                                     \
        if (!(a)) {                                                         \
                TC_IOT_LOG_ERROR("%s CHECK FALSE, return %s(%d)", #a, #ret,  ret);    \
            return ret;                                                      \
        }                                                                    \
    } while (0)

/**< 上报evnet 事件
 {
    "method": "post_event",
    "passthrough": {
        "sid": "c58a000e"
    },
    "state": {
        "reported": {
            "OpenDoor": [{
                    "unlock_ts": 1535363868,
                    "type": 1,
                    "user": 3
                },
                {
                    "unlock_ts": 1535364966,
                    "type": 2,
                    "user": 255
                }
            ]
        }
    }
}
 *
 * */
#define TC_IOT_MQTT_METHOD_POST_EVENT    "post_event"

/**< 上报 错误告警
{
    "method": "raise_error",
    "passthrough": {
        "sid": "c58a000e"
    },
    "state": {
        "reported": {
            "PowerAlarm": 19
        }
    }
}
 *
 * */
#define TC_IOT_MQTT_METHOD_RAISE_ERROR    "raise_error"

/**< 清除错误告警
{
    "method":  "clear_error",
    "passthrough": {
        "sid": "c58a000f"
    },
    "state": {
        "reported": {
            "PowerAlarm": null
        }
    }
}
 *
 * */
#define TC_IOT_MQTT_METHOD_CLEAR_ERROR    "clear_error"

/**
 * @brief tc_iot_clear_error 清除告警。
 *
 * @param c 设备影子对象
 * @param error_name 告警/错误的名称
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_clear_error(tc_iot_shadow_client * c, const char *error_name);

/**
 * @brief tc_iot_report_multi_event_error 上报多个事件/错误, 注意事件/错误不能为复合类型。
 *
 * @param c 设备影子对象
 * @param method 上报的方法,  只能为 TC_IOT_MQTT_METHOD_RAISE_ERROR || TC_IOT_MQTT_METHOD_POST_EVENT
 * @param event_count 上报错误/事件的个数
 * @param  p_fields 待上报错误/事件属性信息数组
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_report_multi_event_error(tc_iot_shadow_client* c, const char* method, int event_count, tc_iot_shadow_property_def * p_fields) ;


/**
 * @brief tc_iot_report_event_raw 上报事件, 一般用于复合事件的数组的上报
 *
 * @param c 设备影子对象
 * @param raw_json 上报事件的内容, 为json数据
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_report_event_raw(tc_iot_shadow_client* c, const char* raw_json);

/**
 * @brief tc_iot_report_event_obj 上报单个事件(复合类型)
 *
 * @param c 设备影子对象
 * @param event_name 上报事件的名称
 * @param method 上报的方法,  只能为 TC_IOT_MQTT_METHOD_RAISE_ERROR || TC_IOT_MQTT_METHOD_POST_EVENT
 * @param event_count 上报事件的子属性/字段个数
 * @param  p_fields 待上报事件字段属性信息数组
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_report_event_obj(tc_iot_shadow_client* c, const char* event_name, const char* method,  int field_count, tc_iot_shadow_property_def * p_fields) ;



#endif