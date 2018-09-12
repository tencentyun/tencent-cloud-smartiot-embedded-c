#include "tc_iot_inc.h"


/**
 * @brief _tc_iot_get_message_ack_callback shadow_get 回调函数
 *
 * @param ack_status 回调状态，标识消息是否正常收到响应，还是已经超时等。
 * @param md 回调状态为 TC_IOT_ACK_SUCCESS 时，用来传递影子数据请求响应消息。
 * @param session_context 回调 context。
 */
void _tc_iot_get_message_ack_callback(tc_iot_command_ack_status_e ack_status, tc_iot_message_data * md , void * session_context) {

    /* tc_iot_mqtt_message* message = NULL; */

    if (ack_status != TC_IOT_ACK_SUCCESS) {
        if (ack_status == TC_IOT_ACK_TIMEOUT) {
            TC_IOT_LOG_ERROR("request timeout");
        }
        return;
    }

    /* message = md->message; */
    tc_iot_device_on_message_received(md);
}

/**
 * @brief _tc_iot_report_message_ack_callback shadow_update 上报消息回调
 *
 * @param ack_status 回调状态，标识消息是否正常收到响应，还是已经超时等。
 * @param md 回调状态为 TC_IOT_ACK_SUCCESS 时，用来传递影子数据请求响应消息。
 * @param session_context 回调 context。
 */
void _tc_iot_report_message_ack_callback(tc_iot_command_ack_status_e ack_status,
        tc_iot_message_data * md , void * session_context) {
    tc_iot_mqtt_message* message = NULL;

    if (ack_status != TC_IOT_ACK_SUCCESS) {
        if (ack_status == TC_IOT_ACK_TIMEOUT) {
            TC_IOT_LOG_ERROR("request timeout");
        } else {
            TC_IOT_LOG_ERROR("request return ack_status=%d", ack_status);
        }
        return;
    }

    message = md->message;
    TC_IOT_LOG_TRACE("[s->c] %s", (char*)message->payload);
}

/**
 * @brief _tc_iot_update_firm_message_ack_callback shadow_update 上报消息回调
 *
 * @param ack_status 回调状态，标识消息是否正常收到响应，还是已经超时等。
 * @param md 回调状态为 TC_IOT_ACK_SUCCESS 时，用来传递影子数据请求响应消息。
 * @param session_context 回调 context。
 */
void _tc_iot_update_firm_message_ack_callback(tc_iot_command_ack_status_e ack_status,
        tc_iot_message_data * md , void * session_context) {
    tc_iot_mqtt_message* message = NULL;

    if (ack_status != TC_IOT_ACK_SUCCESS) {
        if (ack_status == TC_IOT_ACK_TIMEOUT) {
            TC_IOT_LOG_ERROR("request timeout");
        } else {
            TC_IOT_LOG_ERROR("request return ack_status=%d", ack_status);
        }
        return;
    }

    message = md->message;
    TC_IOT_LOG_TRACE("[s->c] %s", (char*)message->payload);
}


/**
 * @brief _tc_iot_sync_shadow_property 根据服务端下发的影子数据，同步到本地设备状态数据，并进
 * 行上报。
 *
 * @param property_count 设备状态字段数
 * @param properties 设备状态数据
 * @param reported 同步数据类型，为 true 表示同步 reported 数据，为
 * false 表示同步 current 数据。
 * @param doc_start reported or desired 数据起始位置。
 * @param json_token json token 数组起始位置
 * @param tok_count 有效 json token 数量
 *
 * @return
 */
int _tc_iot_sync_shadow_property(tc_iot_shadow_client * p_shadow_client,
        int property_total, tc_iot_shadow_property_def * properties, bool reported,
        const char * doc_start, jsmntok_t * json_token, int tok_count) {
    int i,j;
    jsmntok_t  * key_tok = NULL;
    jsmntok_t  * val_tok = NULL;
    char field_buf[TC_IOT_MAX_FIELD_LEN];
    tc_iot_shadow_number new_number = 0;
    tc_iot_shadow_bool new_bool = 0;
    tc_iot_shadow_enum new_enum = 0;
    tc_iot_shadow_int  new_int = 0;
    int  key_len = 0, val_len = 0;
    const char * key_start;
    const char * val_start;
    int ret = 0;
    tc_iot_shadow_property_def * p_prop = NULL;
    void  * ptr = NULL;

    if (!properties) {
        TC_IOT_LOG_ERROR("properties is null");
        return TC_IOT_NULL_POINTER;
    }

    if (!doc_start) {
        TC_IOT_LOG_ERROR("doc_start is null");
        return TC_IOT_NULL_POINTER;
    }

    if (!json_token) {
        TC_IOT_LOG_ERROR("json_token is null");
        return TC_IOT_NULL_POINTER;
    }

    if (!tok_count) {
        TC_IOT_LOG_ERROR("tok_count is invalid");
        return TC_IOT_INVALID_PARAMETER;
    }

    memset(field_buf, 0, sizeof(field_buf));

    for (i = 0; i < tok_count/2; i++) {
        /* 位置 0 是object对象，所以要从位置 1 开始取数据*/
        /* 2*i+1 为 key 字段，2*i + 2 为 value 字段*/
        key_tok = &(json_token[2*i + 1]);
        key_start = doc_start + key_tok->start;
        key_len = key_tok->end - key_tok->start;

        val_tok = &(json_token[2*i + 2]);
        val_start = doc_start + val_tok->start;
        val_len = val_tok->end - val_tok->start;
        for(j = 0; j < property_total; j++) {
            p_prop = &properties[j];
            if (key_len == strlen(p_prop->name) && strncmp(p_prop->name, key_start, key_len) == 0)  {
                if (val_len < sizeof(field_buf)) {
                    strncpy(field_buf, val_start, val_len);
                    field_buf[val_len] = '\0';
                }

                if ((val_len > 0) && strncmp(TC_IOT_JSON_NULL, field_buf, val_len) == 0) {
                    TC_IOT_LOG_WARN("%s recevied null value.", p_prop->name);
                    continue;
                }

                if (p_prop->type == TC_IOT_SHADOW_TYPE_BOOL) {
                    new_bool = (field_buf[0] != 'f') && (field_buf[0] != '0');
                    ptr = &new_bool;
                    TC_IOT_LOG_TRACE("state change:[%s=%s]", p_prop->name, 
                            (*(tc_iot_shadow_bool *) ptr)? TC_IOT_SHADOW_JSON_TRUE:TC_IOT_SHADOW_JSON_FALSE);
                } else if (p_prop->type == TC_IOT_SHADOW_TYPE_NUMBER) {
                    new_number = atof(field_buf);
                    ptr = &new_number;
                    TC_IOT_LOG_TRACE("state change:[%s=%f]", p_prop->name, (*(tc_iot_shadow_number *) ptr));
                } else if (p_prop->type == TC_IOT_SHADOW_TYPE_ENUM) {
                    new_enum = atoi(field_buf);
                    ptr = &new_enum;
                    TC_IOT_LOG_TRACE("state change:[%s=%d]", p_prop->name, (*(tc_iot_shadow_enum *) ptr));
                } else if (p_prop->type == TC_IOT_SHADOW_TYPE_INT) {
                    new_int = atoi(field_buf);
                    ptr = &new_int;
                    TC_IOT_LOG_TRACE("state change:[%s=%d]", p_prop->name, (*(tc_iot_shadow_int *) ptr));
                } else if (p_prop->type == TC_IOT_SHADOW_TYPE_STRING) {
                    ptr = tc_iot_shadow_save_string_to_cached(p_shadow_client, p_prop->id, val_start, val_len, 
                            p_shadow_client->p_shadow_config->p_desired_device_data);
                    if (ptr) {
                        TC_IOT_LOG_TRACE("state change:[%s=%s]", p_prop->name, (const char *)ptr);
                    }
                } else {
                    TC_IOT_LOG_ERROR("%s type=%d invalid.", p_prop->name, p_prop->type);
                    continue;
                }

                if (p_prop->type != TC_IOT_SHADOW_TYPE_STRING) {
                    tc_iot_shadow_save_to_cached(p_shadow_client, p_prop->id, ptr, p_shadow_client->p_shadow_config->p_desired_device_data);
                }
                TC_IOT_BIT_SET(p_shadow_client->desired_bits, p_prop->id);
                ret = tc_iot_shadow_event_notify(p_shadow_client, TC_IOT_SHADOW_EVENT_SERVER_CONTROL, ptr, p_prop);
                if (ret == TC_IOT_SUCCESS) {
                    tc_iot_shadow_save_to_cached(p_shadow_client, p_prop->id, ptr, p_shadow_client->p_shadow_config->p_current_device_data);
                }
            }
        }
    }
    return tc_iot_confirm_devcie_data(p_shadow_client);
}



/**
 * @brief tc_iot_device_on_message_received 数据回调，处理 shadow_get 获取最新状态，或
 * 者影子服务推送的最新控制指令数据。
 *
 * @param md 影子数据消息
 */
void tc_iot_device_on_message_received(tc_iot_message_data* md) {
    jsmntok_t  json_token[TC_IOT_MAX_JSON_TOKEN_COUNT];
    char field_buf[TC_IOT_MAX_FIELD_LEN];
    int field_index = 0;
    int ret = 0;
    tc_iot_shadow_client * p_shadow_client = TC_IOT_CONTAINER_OF(md->mqtt_client, tc_iot_shadow_client, mqtt_client);

    memset(field_buf, 0, sizeof(field_buf));

    tc_iot_mqtt_message* message = md->message;
    TC_IOT_LOG_TRACE("[s->c] %s", (char*)message->payload);

    /* 有效性检查 */
    ret = tc_iot_json_parse(message->payload, message->payloadlen, json_token, TC_IOT_ARRAY_LENGTH(json_token));
    if (ret <= 0) {
        return ;
    }

    tc_iot_mem_usage_log("json_token[TC_IOT_MAX_JSON_TOKEN_COUNT]", sizeof(json_token), sizeof(json_token[0])*ret);

    field_index = tc_iot_json_find_token((char*)message->payload, json_token, ret, "method", field_buf, sizeof(field_buf));
    if (field_index <= 0 ) {
        TC_IOT_LOG_ERROR("field method not found in JSON: %s", (char*)message->payload);
        return ;
    }

    if (strncmp(TC_IOT_MQTT_METHOD_CONTROL, field_buf, strlen(field_buf)) == 0) {
        TC_IOT_LOG_TRACE("Control data receved.");
    } else if (strncmp(TC_IOT_MQTT_METHOD_REPLY, field_buf, strlen(field_buf)) == 0) {
        TC_IOT_LOG_TRACE("Reply pack recevied.");
    }

    tc_iot_shadow_doc_parse(p_shadow_client, (const char *)message->payload, json_token, ret, field_buf, sizeof(field_buf));
}

int tc_iot_shadow_doc_parse(tc_iot_shadow_client * p_shadow_client,
        const char * payload, jsmntok_t * json_token, int token_count, char * field_buf, int field_buf_len) {

    const char * reported_start = NULL;
    int reported_len = 0;
    const char * desired_start = NULL;
    int desired_len = 0;
    int field_index = 0;
    int ret = 0;
    unsigned int seq = 0;
    const char * pos = NULL;

    /* 检查 reported 字段是否存在 */
    field_index = tc_iot_json_find_token(payload, json_token, token_count, "payload.state.reported", NULL, 0);
    if (field_index <= 0 ) {
        /* TC_IOT_LOG_TRACE("payload.state.reported not found"); */
    } else {
        reported_start = payload + json_token[field_index].start;
        reported_len = json_token[field_index].end - json_token[field_index].start;
        TC_IOT_LOG_TRACE("payload.state.reported found:%s", tc_iot_log_summary_string(reported_start, reported_len));
    }

    /* 检查 desired 字段是否存在 */
    field_index = tc_iot_json_find_token(payload, json_token, token_count, "payload.state.desired", NULL, 0);
    if (field_index <= 0 ) {
        /* TC_IOT_LOG_TRACE("payload.state.desired not found"); */
    } else {
        desired_start = payload + json_token[field_index].start;
        desired_len = json_token[field_index].end - json_token[field_index].start;
        TC_IOT_LOG_TRACE("payload.state.desired found:%s", tc_iot_log_summary_string(desired_start, desired_len));

        field_index = tc_iot_json_find_token(payload, json_token, token_count, TC_IOT_SHADOW_SEQUENCE_FIELD, NULL, 0);
        if (field_index > 0) {
            pos = payload + json_token[field_index].start;
            seq = 0;
            while (*pos >= '0' && *pos <= '9') {
                seq = 10*seq + (*pos - '0');
                /* TC_IOT_LOG_TRACE("pos=%c", *pos); */
                pos++;
            }
            if (seq > 0) {
                if (p_shadow_client->shadow_seq > seq ) {
                    TC_IOT_LOG_WARN("seq reversed: old=%u,new=%u ", p_shadow_client->shadow_seq , seq);
                }
                p_shadow_client->shadow_seq = seq;
            }
        } else {
            TC_IOT_LOG_TRACE("no version field.");
        }
    }

    /* 根据控制台或者 APP 端的指令，设定设备状态 */
    if (desired_start) {
        ret = tc_iot_json_parse(desired_start, desired_len, json_token, token_count);
        if (ret <= 0) {
            return TC_IOT_FAILURE;
        }

        TC_IOT_LOG_TRACE("---synchronizing desired status---");
        ret = _tc_iot_sync_shadow_property(
                p_shadow_client,
                p_shadow_client->p_shadow_config->property_total,
                p_shadow_client->p_shadow_config->properties, false,
                desired_start, json_token, ret);
        if (ret == TC_IOT_SUCCESS)  {
            TC_IOT_LOG_TRACE("---synchronizing desired status success---");
        } else {
            TC_IOT_LOG_ERROR("---synchronizing desired status failed, ret=%d---", ret);
        }
    }
    return TC_IOT_SUCCESS;
}

int tc_iot_report_device_data(tc_iot_shadow_client* p_shadow_client) {
    char buffer[TC_IOT_REPORT_UPDATE_MSG_LEN];
    int buffer_len = sizeof(buffer);
    int ret = 0;

    ret = tc_iot_shadow_check_and_report(p_shadow_client, buffer, buffer_len,
            _tc_iot_report_message_ack_callback, p_shadow_client->mqtt_client.command_timeout_ms, NULL, false);
    if (TC_IOT_BUFFER_OVERFLOW == ret) {
        TC_IOT_LOG_ERROR("buffer overflow, please check TC_IOT_REPORT_UPDATE_MSG_LEN");
    } else if (TC_IOT_REPORT_SKIPPED_FOR_NO_CHANGE == ret) {
        ret = TC_IOT_SUCCESS;
    }
    tc_iot_mem_usage_log("buffer[TC_IOT_REPORT_UPDATE_MSG_LEN]", sizeof(buffer), strlen(buffer));
    return ret;
}

int tc_iot_confirm_devcie_data(tc_iot_shadow_client* p_shadow_client) {
    char buffer[TC_IOT_UPDATE_DESIRED_MSG_LEN];
    int buffer_len = sizeof(buffer);
    int ret = 0;

    ret = tc_iot_shadow_check_and_report(p_shadow_client, buffer, buffer_len,
            _tc_iot_report_message_ack_callback, p_shadow_client->mqtt_client.command_timeout_ms, NULL, false);
    tc_iot_mem_usage_log("buffer[TC_IOT_UPDATE_DESIRED_MSG_LEN]", sizeof(buffer), strlen(buffer));
    if ((ret != TC_IOT_SUCCESS) && (ret != TC_IOT_REPORT_SKIPPED_FOR_NO_CHANGE)) {
        if (TC_IOT_BUFFER_OVERFLOW == ret) {
            TC_IOT_LOG_ERROR("buffer overflow, please check TC_IOT_UPDATE_DESIRED_MSG_LEN");
        }
        return ret;
    }
    ret = tc_iot_shadow_check_and_report(p_shadow_client, buffer, buffer_len,
            _tc_iot_report_message_ack_callback, p_shadow_client->mqtt_client.command_timeout_ms, NULL, true);
    if (TC_IOT_BUFFER_OVERFLOW == ret) {
        TC_IOT_LOG_ERROR("buffer overflow, please check TC_IOT_UPDATE_DESIRED_MSG_LEN");
    } else if (TC_IOT_REPORT_SKIPPED_FOR_NO_CHANGE == ret) {
        ret = TC_IOT_SUCCESS;
    }
    return ret;
}

int tc_iot_shadow_event_notify(tc_iot_shadow_client * p_shadow_client, tc_iot_event_e event, void * data, void * context) {
    tc_iot_event_message event_msg;

    if (p_shadow_client
            && p_shadow_client->p_shadow_config
            && p_shadow_client->p_shadow_config->event_notify) {
        event_msg.event = event;
        event_msg.data = data;
        return p_shadow_client->p_shadow_config->event_notify(&event_msg, p_shadow_client, context);
    } else {
        TC_IOT_LOG_TRACE("no event_notify callback, skip calling event_notify.");
        return TC_IOT_SUCCESS;
    }
}

/*
根据初始配置，初始化 client：
1. 连接 MQ 服务器；
2. 订阅 shadow/get/${product_id}/${device_name} Topic，用来接收下行消息；
3. 通知设备应用上报设备属性信息；
4. 发送 {"method":"get"} 请求，获取离线期间累积的，待处理控制指令；
5. 上报最新设备状态；
*/
int tc_iot_server_init(tc_iot_shadow_client* p_shadow_client, tc_iot_shadow_config * p_client_config) {
    int ret = 0;
    char buffer[TC_IOT_GET_MSG_LEN];
    int buffer_len = sizeof(buffer);

    /* 初始化 shadow client */
    TC_IOT_LOG_INFO("constructing mqtt shadow client.");
    ret = tc_iot_shadow_construct(p_shadow_client, p_client_config);
    if (ret != TC_IOT_SUCCESS) {
        return ret;
    }

    TC_IOT_LOG_INFO("construct mqtt shadow client success.");
    TC_IOT_LOG_INFO("yield waiting for server push.");
    /* 执行 yield 收取影子服务端前序指令消息，清理历史状态。 */
    tc_iot_shadow_yield(p_shadow_client, 200);
    TC_IOT_LOG_INFO("yield waiting for server finished.");

    /* 通过get操作主动获取服务端影子设备状态，以便设备端同步更新至最新状态*/
    ret = tc_iot_shadow_get(p_shadow_client, buffer, buffer_len, _tc_iot_get_message_ack_callback,
            p_shadow_client->mqtt_client.command_timeout_ms, p_shadow_client);
    TC_IOT_LOG_TRACE("[c->s] shadow_get%s", buffer);
    tc_iot_mem_usage_log("buffer[TC_IOT_GET_MSG_LEN]", sizeof(buffer), strlen(buffer));

    tc_iot_report_device_data(p_shadow_client);

    return ret;
}

tc_iot_shadow_property_def * tc_iot_shadow_get_property_def(tc_iot_shadow_client * p_shadow_client, int property_id) {
    tc_iot_shadow_property_def * p_prop = NULL;

    if (NULL == p_shadow_client) {
        TC_IOT_LOG_ERROR("p_shadow_client = null");
        return NULL;
    }

    if (property_id < p_shadow_client->p_shadow_config->property_total) {
        p_prop = &p_shadow_client->p_shadow_config->properties[property_id];
        return p_prop;
    } else {
        TC_IOT_LOG_ERROR("invalid property_id=%d, property_total=%d", property_id, p_shadow_client->p_shadow_config->property_total);
        return NULL;
    }
}

const char * tc_iot_shadow_get_property_name(tc_iot_shadow_client * p_shadow_client, int property_id) {
    tc_iot_shadow_property_def * p_prop = NULL;

    if (NULL == p_shadow_client) {
        TC_IOT_LOG_ERROR("p_shadow_client = null");
        return NULL;
    }

    p_prop = tc_iot_shadow_get_property_def(p_shadow_client, property_id);
    if (p_prop) {
        return p_prop->name;
    } else {
        return "not_found";
    }
}

int tc_iot_shadow_get_property_type(tc_iot_shadow_client * p_shadow_client, int property_id) {
    tc_iot_shadow_property_def * p_prop = NULL;

    if (NULL == p_shadow_client) {
        TC_IOT_LOG_ERROR("p_shadow_client = null");
        return TC_IOT_SHADOW_TYPE_INVALID;
    }

    p_prop = tc_iot_shadow_get_property_def(p_shadow_client, property_id);
    if (p_prop) {
        return p_prop->type;
    } else {
        return TC_IOT_SHADOW_TYPE_INVALID;
    }
}

int tc_iot_shadow_get_property_offset(tc_iot_shadow_client * p_shadow_client, int property_id) {
    tc_iot_shadow_property_def * p_prop = NULL;

    if (NULL == p_shadow_client) {
        TC_IOT_LOG_ERROR("p_shadow_client = null");
        return 0;
    }

    p_prop = tc_iot_shadow_get_property_def(p_shadow_client, property_id);
    if (p_prop) {
        return p_prop->offset;
    } else {
        return 0;
    }
}

int tc_iot_shadow_cmp_local(tc_iot_shadow_client * c, int property_id, void * src, void * dest) {
    tc_iot_shadow_property_def * p_prop = NULL;
    void * p_dest_offset = NULL;
    void * p_src_offset = NULL;
    int ret = 0;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(c->p_shadow_config, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(c->p_shadow_config->properties, TC_IOT_NULL_POINTER);

    p_prop = &c->p_shadow_config->properties[property_id];
    p_dest_offset = (char *)dest + p_prop->offset;
    p_src_offset = (char *)src + p_prop->offset;
    switch (p_prop->type) {
        case TC_IOT_SHADOW_TYPE_BOOL:
            ret = memcmp( p_dest_offset, p_src_offset, sizeof(tc_iot_shadow_bool));
            if (0 != ret) {
                TC_IOT_LOG_TRACE("%s differ %s -> %s", p_prop->name,
                        *(tc_iot_shadow_bool*)p_src_offset ? TC_IOT_SHADOW_JSON_TRUE:TC_IOT_SHADOW_JSON_FALSE,
                        *(tc_iot_shadow_bool*)p_dest_offset ? TC_IOT_SHADOW_JSON_TRUE:TC_IOT_SHADOW_JSON_FALSE
                        );
            }
            break;
        case TC_IOT_SHADOW_TYPE_NUMBER:
            ret = memcmp( p_dest_offset, p_src_offset, sizeof(tc_iot_shadow_number));
            if (0 != ret) {
                TC_IOT_LOG_TRACE("%s differ %f -> %f", p_prop->name,
                        *(tc_iot_shadow_number*)p_src_offset,
                        *(tc_iot_shadow_number*)p_dest_offset
                        );
            }
            break;
        case TC_IOT_SHADOW_TYPE_ENUM:
            ret = memcmp( p_dest_offset, p_src_offset, sizeof(tc_iot_shadow_enum));
            if (0 != ret) {
                TC_IOT_LOG_TRACE("%s differ %d -> %d", p_prop->name,
                        *(tc_iot_shadow_enum*)p_src_offset,
                        *(tc_iot_shadow_enum*)p_dest_offset
                        );
            }
            break;
        case TC_IOT_SHADOW_TYPE_INT:
            ret = memcmp( p_dest_offset, p_src_offset, sizeof(tc_iot_shadow_int));
            if (0 != ret) {
                TC_IOT_LOG_TRACE("%s differ %d -> %d", p_prop->name,
                        *(tc_iot_shadow_int*)p_src_offset,
                        *(tc_iot_shadow_int*)p_dest_offset
                        );
            }
            break;
        case TC_IOT_SHADOW_TYPE_STRING:
            ret = strcmp( p_dest_offset, p_src_offset);
            if (0 != ret) {
                TC_IOT_LOG_TRACE("%s differ %s -> %s", p_prop->name,
                        (tc_iot_shadow_string)p_src_offset,
                        (tc_iot_shadow_string)p_dest_offset
                        );
            }
            break;
        default:
            TC_IOT_LOG_ERROR("invalid data type=%d found", p_prop->type);
            return -1;
    }
    return ret;
}

int tc_iot_shadow_cmp_reported_with_local(tc_iot_shadow_client * c, int property_id) {

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(c->p_shadow_config, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(c->p_shadow_config->properties, TC_IOT_NULL_POINTER);

    return tc_iot_shadow_cmp_local(c, property_id,
            c->p_shadow_config->p_reported_device_data,
            c->p_shadow_config->p_current_device_data
            );
}


int tc_iot_shadow_cmp_desired_with_local(tc_iot_shadow_client * c, int property_id) {

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(c->p_shadow_config, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(c->p_shadow_config->properties, TC_IOT_NULL_POINTER);

    return tc_iot_shadow_cmp_local(c, property_id,
            c->p_shadow_config->p_desired_device_data,
            c->p_shadow_config->p_current_device_data
            );
}

void * tc_iot_shadow_copy_local_to_reported(tc_iot_shadow_client * c, int property_id) {
    tc_iot_shadow_property_def * p_prop = NULL;
    void * p_current = NULL;
    void * p_reported = NULL;

    IF_NULL_RETURN_DATA(c, p_reported);
    IF_NULL_RETURN_DATA(c->p_shadow_config, p_reported);
    IF_NULL_RETURN_DATA(c->p_shadow_config->properties, p_reported);

    p_prop = &c->p_shadow_config->properties[property_id];
    p_current = (char *)c->p_shadow_config->p_current_device_data + p_prop->offset;
    p_reported = (char *)c->p_shadow_config->p_reported_device_data + p_prop->offset;

    switch (p_prop->type) {
        case TC_IOT_SHADOW_TYPE_BOOL:
            return memcpy( p_reported, p_current, sizeof(tc_iot_shadow_bool));
        case TC_IOT_SHADOW_TYPE_NUMBER:
            return memcpy( p_reported, p_current, sizeof(tc_iot_shadow_number));
        case TC_IOT_SHADOW_TYPE_ENUM:
            return memcpy( p_reported, p_current, sizeof(tc_iot_shadow_enum));
        case TC_IOT_SHADOW_TYPE_STRING:
            return strcpy( p_reported, p_current);
        default:
            TC_IOT_LOG_ERROR("invalid data type=%d found", p_prop->type);
            return p_reported;
    }
}

void * tc_iot_shadow_save_to_cached(tc_iot_shadow_client * c, int property_id, const void * p_data, void * p_cache) {
    tc_iot_shadow_property_def * p_prop = NULL;
    void * p_dest = NULL;

    IF_NULL_RETURN_DATA(c, p_dest);
    IF_NULL_RETURN_DATA(c->p_shadow_config, p_dest);
    IF_NULL_RETURN_DATA(c->p_shadow_config->properties, p_dest);
    IF_NULL_RETURN_DATA(p_cache, p_dest);

    p_prop = &c->p_shadow_config->properties[property_id];
    p_dest = (char *)p_cache + p_prop->offset;

    switch (p_prop->type) {
        case TC_IOT_SHADOW_TYPE_BOOL:
            return memcpy( p_dest, p_data, sizeof(tc_iot_shadow_bool));
        case TC_IOT_SHADOW_TYPE_NUMBER:
            return memcpy( p_dest, p_data, sizeof(tc_iot_shadow_number));
        case TC_IOT_SHADOW_TYPE_ENUM:
            return memcpy( p_dest, p_data, sizeof(tc_iot_shadow_enum));
        case TC_IOT_SHADOW_TYPE_STRING:
            return strcpy( p_dest, p_data);
        default:
            TC_IOT_LOG_ERROR("invalid data type=%d found", p_prop->type);
            return p_dest;
    }
}

void * tc_iot_shadow_save_string_to_cached(tc_iot_shadow_client * c, int property_id, const void * p_data, int len, void * p_cache) {
    tc_iot_shadow_property_def * p_prop = NULL;
    void * p_dest = NULL;

    IF_NULL_RETURN_DATA(c, p_dest);
    IF_NULL_RETURN_DATA(c->p_shadow_config, p_dest);
    IF_NULL_RETURN_DATA(c->p_shadow_config->properties, p_dest);
    IF_NULL_RETURN_DATA(p_cache, p_dest);

    p_prop = &c->p_shadow_config->properties[property_id];
    p_dest = (char *)p_cache + p_prop->offset;

    if (len >= p_prop->len) {
        TC_IOT_LOG_ERROR("source data too long len=%d, field %s max len=%d", len,p_prop->name, p_prop->len-1);
        return p_dest;
    }

    switch (p_prop->type) {
        case TC_IOT_SHADOW_TYPE_STRING:
            memcpy( p_dest, p_data, len);
            ((char *)(p_dest))[len] = '\0';
            return p_dest;
        default:
            TC_IOT_LOG_ERROR("invalid data type=%d found", p_prop->type);
            return p_dest;
    }
}

int tc_iot_shadow_report_property(tc_iot_shadow_client * c, int property_id, tc_iot_json_writer * w) {
    tc_iot_shadow_property_def * p_prop = NULL;
    void * p_current = NULL;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(c->p_shadow_config, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(c->p_shadow_config->properties, TC_IOT_NULL_POINTER);

    p_prop = &c->p_shadow_config->properties[property_id];
    p_current = (char *)c->p_shadow_config->p_current_device_data + p_prop->offset;
    /* p_reported = (char *)c->p_shadow_config->p_reported_device_data + p_prop->offset; */

    switch (p_prop->type) {
        case TC_IOT_SHADOW_TYPE_BOOL:
            tc_iot_shadow_copy_local_to_reported(c, property_id);
            /* bool 类型数据，在数据模板中实际是以0/1表示。*/
            return tc_iot_json_writer_raw_data(w, p_prop->name, *(tc_iot_shadow_bool *)p_current?TC_IOT_SHADOW_JSON_TRUE:TC_IOT_SHADOW_JSON_FALSE);
        case TC_IOT_SHADOW_TYPE_NUMBER:
            tc_iot_shadow_copy_local_to_reported(c, property_id);
            return tc_iot_json_writer_decimal(w, p_prop->name, *(tc_iot_shadow_number *)p_current);
        case TC_IOT_SHADOW_TYPE_ENUM:
            tc_iot_shadow_copy_local_to_reported(c, property_id);
            return tc_iot_json_writer_int(w, p_prop->name, *(tc_iot_shadow_enum *)p_current);
        case TC_IOT_SHADOW_TYPE_INT:
            tc_iot_shadow_copy_local_to_reported(c, property_id);
            return tc_iot_json_writer_int(w, p_prop->name, *(tc_iot_shadow_int *)p_current);
        case TC_IOT_SHADOW_TYPE_STRING:
            tc_iot_shadow_copy_local_to_reported(c, property_id);
            return tc_iot_json_writer_string(w, p_prop->name, p_current);
        default:
            TC_IOT_LOG_ERROR("invalid data name=%s,type=%d found", p_prop->name, p_prop->type);
            return 0;
    }
}

int tc_iot_shadow_confirm_change(tc_iot_shadow_client * c, int property_id, tc_iot_json_writer * w) {
    tc_iot_shadow_property_def * p_prop = NULL;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(c->p_shadow_config, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(c->p_shadow_config->properties, TC_IOT_NULL_POINTER);

    p_prop = &c->p_shadow_config->properties[property_id];
    return tc_iot_json_writer_null(w,p_prop->name);
}


int tc_iot_shadow_check_and_report(tc_iot_shadow_client *c, char * buffer, int buffer_len,
        message_ack_handler callback, int timeout_ms, void * session_context, bool do_confirm) {
    char *pub_topic ;
    int rc ;
    tc_iot_shadow_session * p_session;
    tc_iot_mqtt_message pubmsg;

    int sid_len;
    tc_iot_json_writer writer;
    tc_iot_json_writer * w = &writer;


    int ret = 0;
    int i = 0;
    int pos = 0;

    int desired_count = 0;
    int reported_count = 0;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);

    p_session = tc_iot_find_empty_session(c);
    if (!p_session) {
        TC_IOT_LOG_ERROR("no more empty session.");
        return TC_IOT_SHADOW_SESSION_NOT_ENOUGH;
    }

    if (do_confirm) {
        tc_iot_json_writer_open(w, buffer, buffer_len);
        tc_iot_json_writer_string(w ,"method", TC_IOT_MQTT_METHOD_DELETE);
    } else {
        tc_iot_json_writer_open(w, buffer, buffer_len);
        tc_iot_json_writer_string(w ,"method", TC_IOT_MQTT_METHOD_UPDATE);
    }

    sid_len = _tc_iot_generate_session_id(&(p_session->sid[0]), TC_IOT_SESSION_ID_LEN+1, &(c->mqtt_client));
    if (sid_len <= 0) {
        TC_IOT_LOG_ERROR("generate session id failed: sid_len=%d", sid_len);
        memset(&(p_session->sid[0]), '0', TC_IOT_SESSION_ID_LEN);
        sid_len = TC_IOT_SESSION_ID_LEN;
        return TC_IOT_SHADOW_SESSION_NOT_ENOUGH;
    } else {
        tc_iot_json_writer_object_begin(w ,"passthrough");
        
        tc_iot_json_writer_string(w ,"sid", &(p_session->sid[0]));
        tc_iot_json_writer_object_end(w);
    }

    if (do_confirm) {
        tc_iot_json_writer_uint(w ,TC_IOT_SHADOW_SEQUENCE_FIELD, c->shadow_seq);
    } else {
        // reported with no version
        // tc_iot_json_writer_uint(w ,"version", c->desired_version);
    }

    tc_iot_json_writer_object_begin(w ,"state");
    if (do_confirm) {
        tc_iot_json_writer_object_begin(w ,"desired");
        for (i = 0; i < c->p_shadow_config->property_total; ++i) {
            /* 清空 desired 数据 */
            if (TC_IOT_BIT_GET(c->desired_bits,i)) {
                /* 仅当本地状态和服务端在控制指令状态一致时，发送确认指令，清空 desired 数据。 */
                if (0 != tc_iot_shadow_cmp_desired_with_local(c, i)) {
                    TC_IOT_LOG_ERROR("device data %s change failure detected(local != desired)", tc_iot_shadow_get_property_name(c,i));
                    continue;
                }
                desired_count++;
                ret = tc_iot_shadow_confirm_change(c,i,w);
                if (ret <= 0) {
                    rc = TC_IOT_BUFFER_OVERFLOW;
                    goto exit;
                }
                TC_IOT_BIT_CLEAR(c->desired_bits, i);
            }
        }
        tc_iot_json_writer_object_end(w);

    } else {
        tc_iot_json_writer_object_begin(w ,"reported");
        for (i = 0; i < c->p_shadow_config->property_total; ++i) {
            /* 未上报过的数据，无条件做上报 */
            if (!TC_IOT_BIT_GET(c->reported_bits,i)) {
                reported_count++;

                ret = tc_iot_shadow_report_property(c, i, w);
                if (ret <= 0) {
                    rc = TC_IOT_BUFFER_OVERFLOW;
                    goto exit;
                }
                TC_IOT_BIT_SET(c->reported_bits,i);
            } else {
                /* 上报过的数据，则对于本地数据和已上报数据不一致的，才做上报 */
                if (tc_iot_shadow_cmp_reported_with_local(c, i) != 0) {
                    reported_count++;
                    ret = tc_iot_shadow_report_property(c, i, w);
                    if (ret <= 0) {
                        rc = TC_IOT_BUFFER_OVERFLOW;
                        goto exit;
                    }
                    TC_IOT_BIT_SET(c->reported_bits,i);
                }
            }
        }
        tc_iot_json_writer_object_end(w);
    }

    tc_iot_json_writer_object_end(w);

    if (desired_count <= 0 && reported_count <= 0) {
        TC_IOT_LOG_TRACE("No device data needed be reported.");
        rc = TC_IOT_REPORT_SKIPPED_FOR_NO_CHANGE;
        goto exit;
    }

    tc_iot_json_writer_object_end(w);
    pos += ret;
    if (pos >= buffer_len) {
        rc = TC_IOT_BUFFER_OVERFLOW;
        goto exit;
    }

    tc_iot_hal_timer_init(&(p_session->timer));
    tc_iot_hal_timer_countdown_ms(&(p_session->timer), timeout_ms);
    p_session->handler = callback;
    p_session->session_context = session_context;

    memset(&pubmsg, 0, sizeof(pubmsg));
    pubmsg.payload = buffer;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = TC_IOT_QOS1;
    pubmsg.retained = 0;
    pubmsg.dup = 0;
    TC_IOT_LOG_TRACE("requesting with: %s", (char *)pubmsg.payload);
    pub_topic = c->p_shadow_config->pub_topic;
    rc = tc_iot_mqtt_client_publish(&(c->mqtt_client), pub_topic, &pubmsg);
    if (TC_IOT_SUCCESS != rc) {
        TC_IOT_LOG_ERROR("tc_iot_mqtt_client_publish failed, return=%d", rc);
    }

exit:
    if (rc != TC_IOT_SUCCESS) {
        tc_iot_release_session(p_session);
    }
    return rc;
}



/*
服务主循环：
1. 接收服务端响应消息和控制指令等；
2. 检查 session 队列请求的超时情况，超时则回调通知应用；
3. 检查keep alive 情况，超时则发送keep alive ping 包；心跳失败，则进入重连；
 * */
int tc_iot_server_loop(tc_iot_shadow_client* p_shadow_client, int yield_timeout) {
    return tc_iot_shadow_yield(p_shadow_client, yield_timeout);
}

/*
服务退出，释放关闭连接、相关资源等。
 * */
int tc_iot_server_destroy(tc_iot_shadow_client* p_shadow_client) {

    TC_IOT_LOG_TRACE("Stopping");
    tc_iot_shadow_destroy(p_shadow_client);
    TC_IOT_LOG_TRACE("Exit success.");
    return 0;
}

