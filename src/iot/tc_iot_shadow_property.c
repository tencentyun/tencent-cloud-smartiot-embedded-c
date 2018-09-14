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
 * @param reported 同步数据类型，为 true 表示同步 reported 数据，为
 * false 表示同步 current 数据。
 * @param doc_start reported or desired 数据起始位置。
 * @param json_token json token 数组起始位置
 * @param tok_count 有效 json token 数量
 *
 * @return
 */
int _tc_iot_sync_shadow_property(tc_iot_shadow_client * p_shadow_client, bool reported, char * doc_start, jsmntok_t * json_token, int tok_count) {
    int i;
    jsmntok_t  * key_tok = NULL;
    jsmntok_t  * val_tok = NULL;
    int  key_len = 0, val_len = 0;
    char * key_start;
    char * val_start;
    tc_iot_shadow_property_def * p_prop = NULL;
    char key_placed = '"';
    char val_placed = '"';

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

    tc_iot_shadow_event_notify(p_shadow_client, TC_IOT_SHADOW_EVENT_BEFORE_SERVER_CONTROL, NULL, NULL);

    for (i = 0; i < tok_count/2; i++) {
        /* 位置 0 是object对象，所以要从位置 1 开始取数据*/
        /* 2*i+1 为 key 字段，2*i + 2 为 value 字段*/
        key_tok = &(json_token[2*i + 1]);
        key_start = doc_start + key_tok->start;
        key_len = key_tok->end - key_tok->start;
        key_placed = key_start[key_len];
        key_start[key_len] = '\0';

        val_tok = &(json_token[2*i + 2]);
        val_start = doc_start + val_tok->start;
        val_len = val_tok->end - val_tok->start;

        val_placed = val_start[val_len];
        val_start[val_len] = '\0';

        if ((val_len > 0) && strcmp(TC_IOT_JSON_NULL, val_start) == 0) {
            TC_IOT_LOG_WARN("skip for %s recevied null value.", p_prop->name);
            continue;
        }

        tc_iot_shadow_event_notify(p_shadow_client, TC_IOT_SHADOW_EVENT_SERVER_CONTROL, val_start, key_start);

        // restore received package
        key_start[key_len] = key_placed;
        val_start[val_len] = val_placed;
    }

    tc_iot_shadow_event_notify(p_shadow_client, TC_IOT_SHADOW_EVENT_AFTER_SERVER_CONTROL, NULL, NULL);

    return TC_IOT_SUCCESS;
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
        TC_IOT_LOG_TRACE("[%s] pack recevied.", field_buf);
        tc_iot_shadow_doc_parse(p_shadow_client, (char *)message->payload, json_token, ret, field_buf, sizeof(field_buf));
    } else if (strncmp(TC_IOT_MQTT_METHOD_REPLY, field_buf, strlen(field_buf)) == 0) {
        TC_IOT_LOG_TRACE("[%s] pack recevied.", field_buf);
        tc_iot_shadow_doc_parse(p_shadow_client, (char *)message->payload, json_token, ret, field_buf, sizeof(field_buf));
    } else if (strncmp(TC_IOT_MQTT_METHOD_REMOTE_CONF, field_buf, strlen(field_buf)) == 0) {
        TC_IOT_LOG_TRACE("[%s] pack recevied.", field_buf);
        tc_iot_shadow_remote_conf_parse(p_shadow_client, (char *)message->payload, json_token, ret, field_buf, sizeof(field_buf));
    } else {
        TC_IOT_LOG_ERROR("unknown method [%s] pack recevied.", field_buf);
        return ;
    }

}

int tc_iot_shadow_remote_conf_parse(tc_iot_shadow_client * p_shadow_client,
        char * payload, jsmntok_t * json_token, int token_count, char * field_buf, int field_buf_len) {
    char * conf_start = NULL;
    int conf_len = 0;
    int field_index = 0;
    int tok_count = 0;
    int ret = 0;
    int i = 0;
    jsmntok_t  * key_tok = NULL;
    jsmntok_t  * val_tok = NULL;
    int  key_len = 0, val_len = 0;
    char * key_start;
    char * val_start;
    tc_iot_shadow_property_def * p_prop = NULL;
    char key_placed = '"';
    char val_placed = '"';

    /* 检查 reported 字段是否存在 */
    field_index = tc_iot_json_find_token(payload, json_token, token_count, "state", NULL, 0);
    if (field_index <= 0 ) {
        TC_IOT_LOG_ERROR("state not found");
        return TC_IOT_FAILURE;
    }

    conf_start = payload + json_token[field_index].start;
    conf_len = json_token[field_index].end - json_token[field_index].start;

    TC_IOT_LOG_TRACE("state found:%s", tc_iot_log_summary_string(conf_start, conf_len));

    ret = tc_iot_json_parse(conf_start, conf_len, json_token, token_count);
    if (ret <= 0) {
        TC_IOT_LOG_ERROR("state parse failed found");
        return ret;
    }

    tok_count = ret;

    for (i = 0; i < tok_count/2; i++) {
        /* 位置 0 是object对象，所以要从位置 1 开始取数据*/
        /* 2*i+1 为 key 字段，2*i + 2 为 value 字段*/
        key_tok = &(json_token[2*i + 1]);
        key_start = conf_start + key_tok->start;
        key_len = key_tok->end - key_tok->start;
        key_placed = key_start[key_len];
        key_start[key_len] = '\0';

        val_tok = &(json_token[2*i + 2]);
        val_start = conf_start + val_tok->start;
        val_len = val_tok->end - val_tok->start;

        val_placed = val_start[val_len];
        val_start[val_len] = '\0';

        if ((val_len > 0) && strcmp(TC_IOT_JSON_NULL, val_start) == 0) {
            TC_IOT_LOG_WARN("skip for %s recevied null value.", p_prop->name);
            continue;
        }

        tc_iot_shadow_event_notify(p_shadow_client, TC_IOT_SHADOW_EVENT_REMOTE_CONF, val_start, key_start);

        // restore received package
        key_start[key_len] = key_placed;
        val_start[val_len] = val_placed;
    }

    return TC_IOT_SUCCESS;
}

int tc_iot_shadow_doc_parse(tc_iot_shadow_client * p_shadow_client,
        char * payload, jsmntok_t * json_token, int token_count, char * field_buf, int field_buf_len) {

    char * reported_start = NULL;
    int reported_len = 0;
    char * desired_start = NULL;
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
                p_shadow_client, false,
                desired_start, json_token, ret);
        if (ret == TC_IOT_SUCCESS)  {
            TC_IOT_LOG_TRACE("---synchronizing desired status success---");
        } else {
            TC_IOT_LOG_ERROR("---synchronizing desired status failed, ret=%d---", ret);
        }
    }
    return TC_IOT_SUCCESS;
}

int tc_iot_report_device_data(tc_iot_shadow_client* c, int count, tc_iot_shadow_property_def * p_fields) {
    char buffer[TC_IOT_REPORT_UPDATE_MSG_LEN];
    int buffer_len = sizeof(buffer);
    int ret = 0;

    ret =  tc_iot_shadow_up_cmd(c, buffer, buffer_len,
                                    TC_IOT_MQTT_METHOD_UPDATE, count, p_fields,
                                    _tc_iot_report_message_ack_callback, c->mqtt_client.command_timeout_ms, NULL);
    tc_iot_mem_usage_log("buffer[TC_IOT_REPORT_UPDATE_MSG_LEN]", sizeof(buffer), strlen(buffer));

    if (TC_IOT_BUFFER_OVERFLOW == ret) {
        TC_IOT_LOG_ERROR("buffer overflow, please check TC_IOT_REPORT_UPDATE_MSG_LEN");
    } else if ((ret > 0) || (TC_IOT_REPORT_SKIPPED_FOR_NO_CHANGE == ret)) {
        ret = TC_IOT_SUCCESS;
    }

    return ret;
}

int tc_iot_confirm_device_data(tc_iot_shadow_client* c, int count, tc_iot_shadow_property_def * p_fields) {
    int ret = 0;
    char buffer[TC_IOT_REPORT_UPDATE_MSG_LEN];
    int buffer_len = sizeof(buffer);

    ret =  tc_iot_shadow_up_cmd(c, buffer, buffer_len,
                                    TC_IOT_MQTT_METHOD_DELETE, count, p_fields,
                                    _tc_iot_report_message_ack_callback, c->mqtt_client.command_timeout_ms, NULL);
    tc_iot_mem_usage_log("buffer[TC_IOT_REPORT_UPDATE_MSG_LEN]", sizeof(buffer), strlen(buffer));

    if (TC_IOT_BUFFER_OVERFLOW == ret) {
        TC_IOT_LOG_ERROR("buffer overflow, please check TC_IOT_REPORT_UPDATE_MSG_LEN");
    } else if ((ret > 0) || (TC_IOT_REPORT_SKIPPED_FOR_NO_CHANGE == ret)) {
        ret = TC_IOT_SUCCESS;
    }

    return ret;
}

int tc_iot_shadow_event_notify(tc_iot_shadow_client * p_shadow_client, tc_iot_event_e event, void * data, void * context) {
    if (p_shadow_client) {
        return tc_iot_mqtt_event_handler(&p_shadow_client->mqtt_client, event, data, context);
    } else {
        TC_IOT_LOG_TRACE("no event_notify callback, skip calling event_notify.");
        return TC_IOT_SUCCESS;
    }
}

int tc_iot_update_firm_info(tc_iot_shadow_client * c) {
    char buffer[TC_IOT_REPORT_UPDATE_MSG_LEN];
    int buffer_len = sizeof(buffer);
    int ret = 0;
    int i = 0;

    tc_iot_shadow_property_def fields[] = {
        {"type", TC_IOT_SHADOW_TYPE_STRING, "wifi"},
        {"hw_id", TC_IOT_SHADOW_TYPE_STRING, "00-00-00-00"},
        {"module", TC_IOT_SHADOW_TYPE_STRING, "unkown"},
        {"module_ver", TC_IOT_SHADOW_TYPE_STRING, "V1.0"},
        {"firm_ver", TC_IOT_SHADOW_TYPE_STRING, "V1.0"},
        {"lat", TC_IOT_SHADOW_TYPE_RAW, "0.0"},
        {"lon", TC_IOT_SHADOW_TYPE_RAW, "0.0"},
        {"keepalive", TC_IOT_SHADOW_TYPE_RAW, "60"},
        {"log_level", TC_IOT_SHADOW_TYPE_RAW, "0"},
        {"is_up_busilog", TC_IOT_SHADOW_TYPE_RAW, "0"},
    };

    for (i = 0; i < TC_IOT_ARRAY_LENGTH(fields); i++) {
        fields[i].value = (void *)tc_iot_hal_get_config(TC_IOT_DCFG_TYPE+i, NULL,0, fields[i].value);
        TC_IOT_LOG_TRACE("value=%s", fields[i].value);
    }

    ret =  tc_iot_shadow_up_cmd(c, buffer, buffer_len,
                                TC_IOT_MQTT_METHOD_UPDATE_FIRM_INFO, TC_IOT_ARRAY_LENGTH(fields), fields,
                                _tc_iot_report_message_ack_callback, c->mqtt_client.command_timeout_ms, NULL);
    tc_iot_mem_usage_log("buffer[TC_IOT_REPORT_UPDATE_MSG_LEN]", sizeof(buffer), strlen(buffer));

    if (TC_IOT_BUFFER_OVERFLOW == ret) {
        TC_IOT_LOG_ERROR("buffer overflow, please check TC_IOT_REPORT_UPDATE_MSG_LEN");
    } else if ((ret > 0) || (TC_IOT_REPORT_SKIPPED_FOR_NO_CHANGE == ret)) {
        ret = TC_IOT_SUCCESS;
    }

    return ret;
}

/*
根据初始配置，初始化 client：
1. 连接 MQ 服务器；
2. 订阅 shadow/get/${product_id}/${device_name} Topic，用来接收下行消息；
3. 通知设备应用上报设备属性信息；
4. 发送 {"method":"get"} 请求，获取离线期间累积的，待处理控制指令；
5. 上报最新设备状态；
*/
int tc_iot_data_template_init(tc_iot_shadow_client* p_shadow_client, tc_iot_shadow_config * p_client_config) {
    int ret = 0;
    /* 初始化 shadow client */
    TC_IOT_LOG_INFO("constructing mqtt shadow client.");
    ret = tc_iot_shadow_construct(p_shadow_client, p_client_config);
    return ret;
}

int tc_iot_data_template_sync(tc_iot_shadow_client* p_shadow_client) {
    int ret = 0;
    char buffer[TC_IOT_GET_MSG_LEN];
    int buffer_len = sizeof(buffer);

    /* 通过get操作主动获取服务端影子设备状态，以便设备端同步更新至最新状态*/
    ret = tc_iot_shadow_get(p_shadow_client, buffer, buffer_len, _tc_iot_get_message_ack_callback,
            p_shadow_client->mqtt_client.command_timeout_ms, p_shadow_client);


    TC_IOT_LOG_TRACE("[c->s] shadow_get%s", buffer);
    tc_iot_mem_usage_log("buffer[TC_IOT_GET_MSG_LEN]", sizeof(buffer), strlen(buffer));

    TC_IOT_LOG_INFO("yield waiting for server push.");
    /* 执行 yield 收取影子服务端前序指令消息，清理历史状态。 */
    tc_iot_shadow_yield(p_shadow_client, 200);
    TC_IOT_LOG_INFO("yield waiting for server finished.");

    /* tc_iot_report_device_data(p_shadow_client); */

    return ret;
}


/*
服务主循环：
1. 接收服务端响应消息和控制指令等；
2. 检查 session 队列请求的超时情况，超时则回调通知应用；
3. 检查keep alive 情况，超时则发送keep alive ping 包；心跳失败，则进入重连；
 * */
int tc_iot_data_template_loop(tc_iot_shadow_client* p_shadow_client, int yield_timeout) {
    return tc_iot_shadow_yield(p_shadow_client, yield_timeout);
}

/*
服务退出，释放关闭连接、相关资源等。
 * */
int tc_iot_data_template_destroy(tc_iot_shadow_client* p_shadow_client) {

    TC_IOT_LOG_TRACE("Stopping");
    tc_iot_shadow_destroy(p_shadow_client);
    TC_IOT_LOG_TRACE("Exit success.");
    return 0;
}

