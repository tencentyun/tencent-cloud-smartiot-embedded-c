#define ENABLE_TC_IOT_LOG_TRACE

#include "tc_iot_inc.h"

tc_iot_shadow_client * tc_iot_get_shadow_client(void);

int tc_iot_event_up_cmd(tc_iot_shadow_client * c, char * buffer, int buffer_len,
                             const char * method, const char* event_name ,  
                             int count, tc_iot_shadow_property_def * p_fields,
                             message_ack_handler callback, int timeout_ms, void * session_context) {
    int ret;
    tc_iot_shadow_session * p_session;
    tc_iot_json_writer writer;
    tc_iot_json_writer * w = &writer;
    int i = 0;
    tc_iot_mqtt_message pubmsg;
    const char * pub_topic;
    

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(buffer, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(method, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(p_fields, TC_IOT_NULL_POINTER);

    p_session = tc_iot_fetch_session(c);
    if (!p_session) {
        TC_IOT_LOG_ERROR("no more empty session.");
        return TC_IOT_SHADOW_SESSION_NOT_ENOUGH;
    }

    tc_iot_json_writer_open(w, buffer, buffer_len);
    tc_iot_json_writer_string(w ,"method", method);

    tc_iot_json_writer_object_begin(w ,"passthrough");
    tc_iot_json_writer_string(w ,"sid", p_session->sid);
    tc_iot_json_writer_object_end(w);

    tc_iot_json_writer_object_begin(w ,"state");

    tc_iot_json_writer_object_begin(w ,"reported");

    tc_iot_json_writer_object_begin(w , event_name);
    for (i = 0; i < count; i++, p_fields++) {
        switch (p_fields->type) {
        case TC_IOT_SHADOW_TYPE_BOOL:
            tc_iot_json_writer_raw_data(w , p_fields->name, *(tc_iot_shadow_bool *)p_fields->value ? TC_IOT_SHADOW_JSON_TRUE:TC_IOT_SHADOW_JSON_FALSE);
            break;
        case TC_IOT_SHADOW_TYPE_INT:
            tc_iot_json_writer_int(w , p_fields->name, *(tc_iot_shadow_int *)p_fields->value);
            break;
        case TC_IOT_SHADOW_TYPE_ENUM:
            tc_iot_json_writer_int(w , p_fields->name, *(tc_iot_shadow_enum *)p_fields->value);
            break;
        case TC_IOT_SHADOW_TYPE_TIMESTAMP:
            tc_iot_json_writer_uint(w , p_fields->name, *(tc_iot_shadow_timestamp *)p_fields->value);
            break;
        case TC_IOT_SHADOW_TYPE_NUMBER:
            tc_iot_json_writer_decimal(w , p_fields->name, *(tc_iot_shadow_number *)p_fields->value);
            break;
        case TC_IOT_SHADOW_TYPE_STRING:
            tc_iot_json_writer_string(w , p_fields->name, (tc_iot_shadow_string)p_fields->value);
            break;
        case TC_IOT_SHADOW_TYPE_RAW:
            tc_iot_json_writer_raw_data(w , p_fields->name, (char *)p_fields->value);
            break;
        default:
            TC_IOT_LOG_ERROR("field type invalid:%s", p_fields->name);
            break;
        }

    }

    tc_iot_json_writer_object_end(w); //end of event_name
    tc_iot_json_writer_object_end(w); //end of reported
    
    tc_iot_json_writer_object_end(w); //end of state

    ret = tc_iot_json_writer_close(w);

    if (ret <= 0) {
        TC_IOT_LOG_ERROR("ret=%d", ret);
        tc_iot_release_session(p_session);
        return ret;
    }

    tc_iot_hal_timer_init(&(p_session->timer));
    tc_iot_hal_timer_countdown_ms(&(p_session->timer), timeout_ms);
    p_session->handler = callback;
    p_session->session_context = session_context;

    pubmsg.payload = buffer;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = TC_IOT_QOS1;
    pubmsg.retained = 0;
    pubmsg.dup = 0;
    TC_IOT_LOG_TRACE("[c-s]: %s", (char *)pubmsg.payload);
    pub_topic = c->p_shadow_config->pub_topic;
    ret = tc_iot_mqtt_client_publish(&(c->mqtt_client), pub_topic, &pubmsg);
    if (TC_IOT_SUCCESS != ret) {
        TC_IOT_LOG_ERROR("tc_iot_mqtt_client_publish failed, return=%d", ret);
    }

    return w->pos;
}

int tc_iot_event_up_cmd_raw(tc_iot_shadow_client * c, char * buffer, int buffer_len,
                             const char* raw_json,
                             message_ack_handler callback, int timeout_ms, void * session_context) {
    int ret;
    tc_iot_shadow_session * p_session;

    
    tc_iot_mqtt_message pubmsg;
    const char * pub_topic;
    
    char buf_tmp[] = "{\"method\":\"post_event\",\"passthrough\":{\"sid\":\"%s\"},\"state\":{\"reported\":%s}}";

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(buffer, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(raw_json, TC_IOT_NULL_POINTER);


    p_session = tc_iot_fetch_session(c);
    if (!p_session) {
        TC_IOT_LOG_ERROR("no more empty session.");
        return TC_IOT_SHADOW_SESSION_NOT_ENOUGH;
    }

    ret = snprintf(buffer, buffer_len, buf_tmp, p_session->sid, raw_json );

    tc_iot_hal_timer_init(&(p_session->timer));
    tc_iot_hal_timer_countdown_ms(&(p_session->timer), timeout_ms);
    p_session->handler = callback;
    p_session->session_context = session_context;

    pubmsg.payload = buffer;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = TC_IOT_QOS1;
    pubmsg.retained = 0;
    pubmsg.dup = 0;
    TC_IOT_LOG_TRACE("[c-s]: %s", (char *)pubmsg.payload);
    pub_topic = c->p_shadow_config->pub_topic;
    ret = tc_iot_mqtt_client_publish(&(c->mqtt_client), pub_topic, &pubmsg);
    if (TC_IOT_SUCCESS != ret) {
        TC_IOT_LOG_ERROR("tc_iot_mqtt_client_publish failed, return=%d", ret);
    }

    return ret;
}

void _tc_iot_report_message_ack_callback(tc_iot_command_ack_status_e ack_status,
        tc_iot_message_data * md , void * session_context) ;

int tc_iot_report_event_obj(tc_iot_shadow_client* c, const char* event_name, const char* method,  int field_count, tc_iot_shadow_property_def * p_fields) {

    int ret = 0;
    char buffer[TC_IOT_REPORT_EVENT_ERROR_MSG_LEN];
    int buffer_len = sizeof(buffer);

    IF_FALSE_RETURN(0== strcmp(method,TC_IOT_MQTT_METHOD_RAISE_ERROR)|| 0==strcmp(method,TC_IOT_MQTT_METHOD_POST_EVENT), -1);

    ret =  tc_iot_event_up_cmd(c, buffer, buffer_len,
                                    TC_IOT_MQTT_METHOD_POST_EVENT, event_name, field_count, p_fields,
                                    _tc_iot_report_message_ack_callback, c->mqtt_client.command_timeout_ms, NULL);
    tc_iot_mem_usage_log("buffer[TC_IOT_REPORT_EVENT_ERROR_MSG_LEN]", sizeof(buffer), strlen(buffer));

    if (TC_IOT_BUFFER_OVERFLOW == ret) {
        TC_IOT_LOG_ERROR("buffer overflow, please check TC_IOT_REPORT_EVENT_ERROR_MSG_LEN");
    } else if ((ret > 0) || (TC_IOT_REPORT_SKIPPED_FOR_NO_CHANGE == ret)) {
        ret = TC_IOT_SUCCESS;
    }

    return ret;

}


int tc_iot_report_event_raw(tc_iot_shadow_client* c, const char* raw_json)
{
    int ret = 0;
    char buffer[TC_IOT_REPORT_EVENT_ERROR_MSG_LEN];
    int buffer_len = sizeof(buffer);


    ret =  tc_iot_event_up_cmd_raw(c, buffer, buffer_len,
                                    raw_json,
                                    _tc_iot_report_message_ack_callback, c->mqtt_client.command_timeout_ms, NULL);
    tc_iot_mem_usage_log("buffer[TC_IOT_REPORT_EVENT_ERROR_MSG_LEN]", sizeof(buffer), strlen(buffer));

    if (TC_IOT_BUFFER_OVERFLOW == ret) {
        TC_IOT_LOG_ERROR("buffer overflow, please check TC_IOT_REPORT_EVENT_ERROR_MSG_LEN");
    } else if ((ret > 0) || (TC_IOT_REPORT_SKIPPED_FOR_NO_CHANGE == ret)) {
        ret = TC_IOT_SUCCESS;
    }

    return ret;
}

int tc_iot_report_multi_event_error(tc_iot_shadow_client* c, const char* method, int event_count, tc_iot_shadow_property_def * p_fields) {

    int ret = 0;
    char buffer[TC_IOT_REPORT_EVENT_ERROR_MSG_LEN];
    int buffer_len = sizeof(buffer);

    IF_FALSE_RETURN( 0== strcmp(method,TC_IOT_MQTT_METHOD_RAISE_ERROR)|| 0==strcmp(method,TC_IOT_MQTT_METHOD_POST_EVENT), -1);

    ret =  tc_iot_shadow_up_cmd(c, buffer, buffer_len,
                                    method, event_count, p_fields,
                                    _tc_iot_report_message_ack_callback, c->mqtt_client.command_timeout_ms, NULL);
    tc_iot_mem_usage_log("buffer[TC_IOT_REPORT_EVENT_ERROR_MSG_LEN]", sizeof(buffer), strlen(buffer));

    if (TC_IOT_BUFFER_OVERFLOW == ret) {
        TC_IOT_LOG_ERROR("buffer overflow, please check TC_IOT_REPORT_EVENT_ERROR_MSG_LEN");
    } else if ((ret > 0) || (TC_IOT_REPORT_SKIPPED_FOR_NO_CHANGE == ret)) {
        ret = TC_IOT_SUCCESS;
    }

    return ret;

}

int tc_iot_report_single_error(tc_iot_shadow_client* c, const char* err_name , tc_iot_shadow_data_type_e error_type,  void * value)
{
	tc_iot_shadow_property_def  def_name = { err_name , error_type , value };
	return tc_iot_report_multi_event_error(tc_iot_get_shadow_client(), TC_IOT_MQTT_METHOD_RAISE_ERROR, 1 , &def_name  );
}

int tc_iot_clear_error_up_cmd(tc_iot_shadow_client * c, char * buffer, int buffer_len,
                             const char* error_name ,  
                             message_ack_handler callback, int timeout_ms, void * session_context) {
    int ret;
    tc_iot_shadow_session * p_session;
    tc_iot_json_writer writer;
    tc_iot_json_writer * w = &writer;
    
    tc_iot_mqtt_message pubmsg;
    const char * pub_topic;
    

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(buffer, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(error_name, TC_IOT_NULL_POINTER);
    

    p_session = tc_iot_fetch_session(c);
    if (!p_session) {
        TC_IOT_LOG_ERROR("no more empty session.");
        return TC_IOT_SHADOW_SESSION_NOT_ENOUGH;
    }

    tc_iot_json_writer_open(w, buffer, buffer_len);
    tc_iot_json_writer_string(w ,"method", TC_IOT_MQTT_METHOD_CLEAR_ERROR);

    tc_iot_json_writer_object_begin(w ,"passthrough");
    tc_iot_json_writer_string(w ,"sid", p_session->sid);
    tc_iot_json_writer_object_end(w);

    tc_iot_json_writer_object_begin(w ,"state");

    tc_iot_json_writer_object_begin(w ,"reported");

    tc_iot_json_writer_raw_data(w , error_name, TC_IOT_JSON_NULL);   


    tc_iot_json_writer_object_end(w); //end of reported
    
    tc_iot_json_writer_object_end(w); //end of state

    ret = tc_iot_json_writer_close(w);

    if (ret <= 0) {
        TC_IOT_LOG_ERROR("ret=%d", ret);
        tc_iot_release_session(p_session);
        return ret;
    }

    tc_iot_hal_timer_init(&(p_session->timer));
    tc_iot_hal_timer_countdown_ms(&(p_session->timer), timeout_ms);
    p_session->handler = callback;
    p_session->session_context = session_context;

    pubmsg.payload = buffer;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = TC_IOT_QOS1;
    pubmsg.retained = 0;
    pubmsg.dup = 0;
    TC_IOT_LOG_TRACE("[c-s]: %s", (char *)pubmsg.payload);
    pub_topic = c->p_shadow_config->pub_topic;
    ret = tc_iot_mqtt_client_publish(&(c->mqtt_client), pub_topic, &pubmsg);
    if (TC_IOT_SUCCESS != ret) {
        TC_IOT_LOG_ERROR("tc_iot_mqtt_client_publish failed, return=%d", ret);
    }

    return w->pos;
}

int tc_iot_clear_error(tc_iot_shadow_client * c, const char *error_name)
{
	int ret = 0;
    char buffer[TC_IOT_REPORT_EVENT_ERROR_MSG_LEN];
    int buffer_len = sizeof(buffer);

    ret =  tc_iot_clear_error_up_cmd(c, buffer, buffer_len, error_name,
                                    _tc_iot_report_message_ack_callback, c->mqtt_client.command_timeout_ms, NULL);
    tc_iot_mem_usage_log("buffer[TC_IOT_REPORT_EVENT_ERROR_MSG_LEN]", sizeof(buffer), strlen(buffer));

    if (TC_IOT_BUFFER_OVERFLOW == ret) {
        TC_IOT_LOG_ERROR("buffer overflow, please check TC_IOT_REPORT_EVENT_ERROR_MSG_LEN");
    } else if ((ret > 0) || (TC_IOT_REPORT_SKIPPED_FOR_NO_CHANGE == ret)) {
        ret = TC_IOT_SUCCESS;
    }

    return ret;
}