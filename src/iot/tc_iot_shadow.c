#include "tc_iot_inc.h"

static void _tc_iot_shadow_on_message_received(tc_iot_message_data *md) {
    jsmntok_t  json_token[TC_IOT_MAX_JSON_TOKEN_COUNT];

    tc_iot_mqtt_message *message = md->message;
    tc_iot_shadow_client *c = md->context;
    char session_id[TC_IOT_SESSION_ID_LEN+1];
    tc_iot_shadow_session * session;
    int field_index = 0;
    int i;
    int ret;

    ret = tc_iot_json_parse(message->payload, message->payloadlen, json_token, TC_IOT_ARRAY_LENGTH(json_token));
    if (ret <= 0) {
        if (TC_IOT_JSON_PARSE_TOKEN_NO_MEM == ret) {
            TC_IOT_LOG_ERROR("change TC_IOT_MAX_JSON_TOKEN_COUNT larger, mem not enough ->%s", (char *)message->payload);
        } else {
            TC_IOT_LOG_ERROR("BADFORMAT ->%s", (char *)message->payload);
        }
        return ;
    }

    tc_iot_mem_usage_log("json_token[TC_IOT_MAX_JSON_TOKEN_COUNT]", sizeof(json_token), sizeof(json_token[0])*ret);

    field_index = tc_iot_json_find_token((char*)message->payload, json_token, ret,
            "passthrough.sid", session_id, sizeof(session_id));
    if (field_index > 0 ) {
        for (i = 0; i < TC_IOT_MAX_SESSION_COUNT; i++) {
            session = &(c->sessions[i]);
            if (session->sid[0] != '\0' && strncmp(session->sid, session_id, strlen(session_id)) == 0) {
                if (session->handler) {
                    /* TC_IOT_LOG_TRACE("session:%s response received", session->sid); */
                    session->handler(TC_IOT_ACK_SUCCESS, md, session->session_context);
                } else {
                    TC_IOT_LOG_ERROR("session:%s handler not found", session->sid);
                }
                tc_iot_release_session(session);
                return ;
            }
        }
    } else {
        TC_IOT_LOG_TRACE("field passthrough.sid not found, could be push from server.");
    }

    if (c && c->p_shadow_config && c->p_shadow_config->on_receive_msg) {
        c->p_shadow_config->on_receive_msg(md);
    } else {
        TC_IOT_LOG_ERROR("UNHANDLED ->%s", (char *)message->payload);
    }
}

int tc_iot_shadow_construct(tc_iot_shadow_client *c,
                            tc_iot_shadow_config *p_cfg) {
    int rc;
    tc_iot_mqtt_client_config *p_config;
    tc_iot_mqtt_client *p_mqtt_client;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(p_cfg, TC_IOT_NULL_POINTER);

    memset(c, 0, sizeof(tc_iot_shadow_client));

    c->p_shadow_config = p_cfg;
    p_config = &(p_cfg->mqtt_client_config);

    p_mqtt_client = &(c->mqtt_client);
    rc = tc_iot_mqtt_client_construct(p_mqtt_client, p_config);
    if (rc != TC_IOT_SUCCESS) {
        return rc;
    }

    rc = tc_iot_mqtt_client_connect(p_mqtt_client);
    if (rc != TC_IOT_SUCCESS) {
        return rc;
    }

    rc = tc_iot_mqtt_client_subscribe(p_mqtt_client, p_cfg->sub_topic, TC_IOT_QOS1,
                                          _tc_iot_shadow_on_message_received, c);
    if (TC_IOT_SUCCESS == rc) {
        TC_IOT_LOG_TRACE("subscribing to %s success.", p_cfg->sub_topic);
    } else {
        TC_IOT_LOG_ERROR("subscribing to %s failed, ret code=%d.", p_cfg->sub_topic,
                  rc);
    }
    return rc;
}


void tc_iot_shadow_destroy(tc_iot_shadow_client *c) {
    if (c) {
        tc_iot_mqtt_client_disconnect(&(c->mqtt_client));
    }
}

char tc_iot_shadow_isconnected(tc_iot_shadow_client *c) {
    IF_NULL_RETURN(c, 0);
    return tc_iot_mqtt_client_is_connected(&(c->mqtt_client));
}

static int _tc_iot_check_expired_session(tc_iot_shadow_client *c) {
    int i;
    tc_iot_shadow_session * session;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);

    for (i = 0; i < TC_IOT_MAX_SESSION_COUNT; i++) {
        session = &(c->sessions[i]);
        if (session->sid[0] != 0) {
            if (tc_iot_hal_timer_is_expired(&(session->timer))) {
                TC_IOT_LOG_WARN("session:%s expired", session->sid);
                if (session->handler) {
                    session->handler(TC_IOT_ACK_TIMEOUT, NULL, session->session_context);
                } else {
                    TC_IOT_LOG_ERROR("session:%s handler not found", session->sid);
                }
                tc_iot_release_session(session);
            } else {
                /* TC_IOT_LOG_TRACE("session:%s not expired, left_ms=%d", session->sid, */
                        /* tc_iot_hal_timer_left_ms(&(session->timer))); */
            }
        }
    }
    return TC_IOT_SUCCESS;
}

int tc_iot_shadow_yield(tc_iot_shadow_client *c, int timeout_ms) {
	int ret;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    ret = tc_iot_mqtt_client_yield(&(c->mqtt_client), timeout_ms);
    _tc_iot_check_expired_session(c);

	return ret;
}

tc_iot_shadow_session * tc_iot_find_empty_session(tc_iot_shadow_client *c) {
    int i;

    if (!c) {
        return NULL;
    }

    for (i = 0; i < TC_IOT_MAX_SESSION_COUNT; i++) {
        if (strlen(c->sessions[i].sid) == 0) {
            return &(c->sessions[i]);
        }
    }

    for (i = 0; i < TC_IOT_MAX_SESSION_COUNT; i++) {
        TC_IOT_LOG_TRACE("occupied sid[%d]:%s", i, c->sessions[i].sid);
    }
    return NULL;
}

int tc_iot_shadow_pending_session_count(tc_iot_shadow_client *c) {
    int i;
    int total = 0;

    if (!c) {
        return 0;
    }

    for (i = 0; i < TC_IOT_MAX_SESSION_COUNT; i++) {
        if (strlen(c->sessions[i].sid) != 0) {
            total++;
        }
    }
    return total;
}

void tc_iot_release_session(tc_iot_shadow_session * p_session) {
    if (!p_session) {
        return ;
    }

    TC_IOT_LOG_TRACE("sid released:%s", p_session->sid);
    memset(p_session, 0, sizeof(tc_iot_shadow_session));
    return ;
}

int tc_iot_shadow_get(tc_iot_shadow_client *c, char * buffer, int buffer_len,
         message_ack_handler callback, int timeout_ms, void * session_context) {
    char *pub_topic ;
    int rc ;
    tc_iot_shadow_session * p_session;
    tc_iot_mqtt_message pubmsg;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);

    if (callback) {
        if (timeout_ms <= 0) {
            TC_IOT_LOG_ERROR("callback handler given, but timeout_ms=%d", timeout_ms);
        }
        p_session = tc_iot_find_empty_session(c);
        if (!p_session) {
            TC_IOT_LOG_ERROR("no more empty session.");
            return TC_IOT_SHADOW_SESSION_NOT_ENOUGH;
        }
        rc = tc_iot_shadow_doc_pack_for_get_with_sid(buffer, buffer_len, &(p_session->sid[0]),
                TC_IOT_SESSION_ID_LEN+1,  false, true,
                c);
        if (rc < 0) {
            TC_IOT_LOG_ERROR("tc_iot_shadow_doc_pack_for_get_with_sid failed, return=%d", rc);
            tc_iot_release_session(p_session);
            return rc;
        }
        tc_iot_hal_timer_init(&(p_session->timer));
        tc_iot_hal_timer_countdown_ms(&(p_session->timer), timeout_ms);
        p_session->handler = callback;
        p_session->session_context = session_context;
    } else {
        rc = tc_iot_shadow_doc_pack_for_get_with_sid(buffer, buffer_len, NULL, 0, false, true, c);
        if (rc < 0) {
            TC_IOT_LOG_ERROR("tc_iot_shadow_doc_pack_for_get_with_sid failed, return=%d", rc);
            return rc;
        }
    }


    memset(&pubmsg, 0, sizeof(pubmsg));
    pubmsg.payload = buffer;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = TC_IOT_QOS1;
    pubmsg.retained = 0;
    pubmsg.dup = 0;
    /*TC_IOT_LOG_TRACE("requesting with: %s", (char *)pubmsg.payload);*/
    pub_topic = c->p_shadow_config->pub_topic;
    rc = tc_iot_mqtt_client_publish(&(c->mqtt_client), pub_topic, &pubmsg);
    if (TC_IOT_SUCCESS != rc) {
        TC_IOT_LOG_ERROR("tc_iot_mqtt_client_publish failed, return=%d", rc);
    }
    return rc;
}

int tc_iot_shadow_update(tc_iot_shadow_client *c, char * buffer, int buffer_len,
        const char * reported, const char * desired,
        message_ack_handler callback, int timeout_ms, void * session_context) {
    char *pub_topic ;
    int rc ;
    tc_iot_shadow_session * p_session;
    tc_iot_mqtt_message pubmsg;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);

    if (callback) {
        if (timeout_ms <= 0) {
            TC_IOT_LOG_ERROR("callback handler given, but timeout_ms=%d", timeout_ms);
        }
        p_session = tc_iot_find_empty_session(c);
        if (!p_session) {
            TC_IOT_LOG_ERROR("no more empty session.");
            return TC_IOT_SHADOW_SESSION_NOT_ENOUGH;
        }
        _tc_iot_generate_session_id( &(p_session->sid[0]),sizeof(p_session->sid), &(c->mqtt_client));
        rc = tc_iot_shadow_doc_pack(buffer, buffer_len, TC_IOT_MQTT_METHOD_UPDATE,
                                    &(p_session->sid[0]), reported, desired, c);
        tc_iot_hal_timer_init(&(p_session->timer));
        tc_iot_hal_timer_countdown_ms(&(p_session->timer), timeout_ms);
        p_session->handler = callback;
        p_session->session_context = session_context;
    } else {
        rc = tc_iot_shadow_doc_pack(buffer, buffer_len, TC_IOT_MQTT_METHOD_UPDATE,
                                    NULL, reported, desired, c);
    }

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
    return rc;
}

static unsigned int  _get_unique_session_id(tc_iot_mqtt_client* c) {
    static unsigned short usid = 0;
    unsigned int ret =  ((c->client_init_time << 16) & 0xFFFF0000) | (0xFFFF & (usid++));
    return ret;
}


int _tc_iot_generate_session_id(char * session_id, int session_id_len, tc_iot_mqtt_client* c) {
    unsigned int sid = _get_unique_session_id(c);
    int ret ;
    ret = tc_iot_hal_snprintf(session_id, session_id_len, "%x", sid);
    if (ret > 0 && ret < session_id_len) {
        session_id[ret] = '\0';
    }
    return ret;
}


int tc_iot_shadow_doc_pack_for_get_with_sid(char *buffer, int buffer_len,
                                    char * session_id, int session_id_len,
                                    bool metadata, bool reported,
                                    tc_iot_shadow_client *c) {
    int ret;
    int sid_len;
    tc_iot_json_writer writer;
    tc_iot_json_writer * w = &writer;

    tc_iot_json_writer_open(w, buffer, buffer_len);
    tc_iot_json_writer_string(w ,"method", TC_IOT_MQTT_METHOD_GET);

    if (session_id && (session_id_len >= TC_IOT_SESSION_ID_LEN)) {
        sid_len = _tc_iot_generate_session_id(session_id, session_id_len, &(c->mqtt_client));
        if (sid_len <= 0) {
            TC_IOT_LOG_ERROR("generate session id failed: sid_len=%d", sid_len);
            memset(session_id, '0', TC_IOT_SESSION_ID_LEN);
            sid_len = TC_IOT_SESSION_ID_LEN;
        } else {
            tc_iot_json_writer_object_begin(w ,"passthrough");
            tc_iot_json_writer_string(w ,"sid", session_id);
            tc_iot_json_writer_object_end(w);
        }
    }

    tc_iot_json_writer_bool(w ,"metadata", metadata);
    tc_iot_json_writer_bool(w ,"reported", reported);
    ret = tc_iot_json_writer_close(w);

    if (ret <= 0) {
        TC_IOT_LOG_INFO("encode json failed ,ret=%d", ret);
    }

    return ret;
}

int tc_iot_shadow_delete(tc_iot_shadow_client *c, char * buffer, int buffer_len,
        const char * reported, const char * desired,
        message_ack_handler callback, int timeout_ms, void * session_context) {
    char *pub_topic ;
    int rc ;
    tc_iot_shadow_session * p_session;
    tc_iot_mqtt_message pubmsg;

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);

    if (callback) {
        if (timeout_ms <= 0) {
            TC_IOT_LOG_ERROR("callback handler given, but timeout_ms=%d", timeout_ms);
        }
        p_session = tc_iot_find_empty_session(c);
        if (!p_session) {
            TC_IOT_LOG_ERROR("no more empty session.");
            return TC_IOT_SHADOW_SESSION_NOT_ENOUGH;
        }
        _tc_iot_generate_session_id( &(p_session->sid[0]),sizeof(p_session->sid), &(c->mqtt_client));
        rc = tc_iot_shadow_doc_pack(buffer, buffer_len, TC_IOT_MQTT_METHOD_DELETE,
                                    &(p_session->sid[0]), reported, desired, c);
        tc_iot_hal_timer_init(&(p_session->timer));
        tc_iot_hal_timer_countdown_ms(&(p_session->timer), timeout_ms);
        p_session->handler = callback;
        p_session->session_context = session_context;
    } else {
        rc = tc_iot_shadow_doc_pack(buffer, buffer_len, TC_IOT_MQTT_METHOD_DELETE,
                                    NULL, reported, desired, c);
    }

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
    return rc;
}


int tc_iot_shadow_doc_pack(char *buffer, int buffer_len, const char * method,
                           const char * session_id,
                           const char * reported, const char * desired,
                           tc_iot_shadow_client *c) {
    int ret;
    tc_iot_json_writer writer;
    tc_iot_json_writer * w = &writer;

    tc_iot_json_writer_open(w, buffer, buffer_len);
    tc_iot_json_writer_string(w ,"method", method);

    if (session_id) {
        tc_iot_json_writer_object_begin(w ,"passthrough");
        tc_iot_json_writer_string(w ,"sid", session_id);
        tc_iot_json_writer_object_end(w);
    }

    tc_iot_json_writer_object_begin(w ,"state");
    if (reported) {
        tc_iot_json_writer_raw_data(w ,"reported", reported);
    }

    if (desired) {
        tc_iot_json_writer_raw_data(w ,"desired", desired);
    }
    tc_iot_json_writer_object_end(w);

    ret = tc_iot_json_writer_close(w);

    if (ret <= 0) {
        TC_IOT_LOG_INFO("encode json failed ,ret=%d", ret);
    }

    return ret;
}


