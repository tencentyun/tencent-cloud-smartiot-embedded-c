#include "tc_iot_inc.h"

int tc_iot_http_api_query(tc_iot_device_info* p_device_info) {
    char http_buffer[TC_IOT_HTTP_QUERY_RESPONSE_LEN];
    char req_form[TC_IOT_HTTP_QUERY_REQUEST_FORM_LEN];
    int req_form_len;
    int ret;
    char* rsp_body;
    tc_iot_http_client *p_http_client, http_client;

    jsmn_parser p;
    jsmntok_t t[20];

    char temp_buf[TC_IOT_HTTP_MAX_URL_LENGTH];
    int returnCodeIndex = 0;
    int r;
    int field_index;
    const int timeout_ms = TC_IOT_API_TIMEOUT_MS;
    bool secured = false;
    uint16_t port = HTTP_DEFAULT_PORT;
#if defined(ENABLE_TLS)
    secured = true;
    port = HTTPS_DEFAULT_PORT;
#endif


    IF_NULL_RETURN(p_device_info, TC_IOT_NULL_POINTER);


    req_form_len = tc_iot_create_query_request_form(
        req_form, sizeof(req_form), p_device_info->product_id
        );

    tc_iot_mem_usage_log("req_form[TC_IOT_HTTP_QUERY_REQUEST_FORM_LEN]", sizeof(req_form), req_form_len);

    TC_IOT_LOG_TRACE("request form:\n%s", req_form);

    p_http_client = &http_client;
    tc_iot_http_client_init(p_http_client, HTTP_POST);
    tc_iot_http_client_set_body(p_http_client, req_form);
    tc_iot_http_client_set_host(p_http_client, p_device_info->http_host);
    tc_iot_http_client_set_abs_path(p_http_client, TC_IOT_API_QUERY_PATH);
    tc_iot_http_client_set_content_type(p_http_client, HTTP_CONTENT_FORM_URLENCODED);

    tc_iot_http_client_format_buffer(http_buffer, sizeof(http_buffer), p_http_client);

    TC_IOT_LOG_TRACE("http_buffer=%s", http_buffer);
    ret = tc_iot_http_client_perform(http_buffer,strlen(http_buffer), sizeof(http_buffer),
                                     p_device_info->http_host, port, secured, timeout_ms);
    tc_iot_mem_usage_log("http_buffer[TC_IOT_HTTP_QUERY_RESPONSE_LEN]", sizeof(http_buffer), strlen(http_buffer));

    if (ret < 0) {
        return ret;
    }

    rsp_body = http_buffer;

    if (rsp_body) {
        jsmn_init(&p);

        TC_IOT_LOG_TRACE("\nbody:\n%s\n", rsp_body);

        r = jsmn_parse(&p, rsp_body, strlen(rsp_body), t,
                       sizeof(t) / sizeof(t[0]));
        if (r < 0) {
            TC_IOT_LOG_ERROR("Failed to parse JSON: %s", rsp_body);
            return TC_IOT_JSON_PARSE_FAILED;
        }

        if (r < 1 || t[0].type != JSMN_OBJECT) {
            TC_IOT_LOG_ERROR("Invalid JSON: %s", rsp_body);
            return TC_IOT_JSON_PARSE_FAILED;
        }

        returnCodeIndex = tc_iot_json_find_token(rsp_body, t, r, "returnCode",
                                                 temp_buf, sizeof(temp_buf));
        if (returnCodeIndex <= 0 || strlen(temp_buf) != 1 ||
            temp_buf[0] != '0') {
            TC_IOT_LOG_ERROR("failed to fetch token %d/%s: %s", returnCodeIndex,
                      temp_buf, rsp_body);
            return TC_IOT_ERROR_HTTP_REQUEST_FAILED;
        }

        field_index = tc_iot_json_find_token(rsp_body, t, r, "data.mqtt_host",
                                                p_device_info->mqtt_host,
                                             sizeof(p_device_info->mqtt_host));
        if (field_index <= 0) {
            TC_IOT_LOG_TRACE("data.mqtt_host not found in response.");
            return TC_IOT_ERROR_HTTP_REQUEST_FAILED;
        }

        /* field_index = tc_iot_json_find_token(rsp_body, t, r, "data.mqtt_ip", */
        /*                                         p_device_info->mqtt_ip, */
        /*                                      sizeof(p_device_info->mqtt_ip)); */
        /* if (field_index <= 0) { */
        /*     TC_IOT_LOG_TRACE("data.mqtt_ip not found in response."); */
        /*     return TC_IOT_ERROR_HTTP_REQUEST_FAILED; */
        /* } */

        return TC_IOT_SUCCESS;
    } else {
        return TC_IOT_ERROR_HTTP_REQUEST_FAILED;
    }
}
