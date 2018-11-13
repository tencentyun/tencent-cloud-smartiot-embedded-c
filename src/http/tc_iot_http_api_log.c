#include "tc_iot_inc.h"

int tc_iot_calc_log_sign(char* sign_out, int max_sign_len,
                         const char* secret, const char* device_name,
                         const char* product_id) {
    unsigned char sha256_digest[TC_IOT_SHA256_DIGEST_SIZE];
    int ret;
    char b64_buf[TC_IOT_BASE64_ENCODE_OUT_LEN(TC_IOT_SHA256_DIGEST_SIZE)];
    int url_ret;

    IF_NULL_RETURN(sign_out, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(secret, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(device_name, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(product_id, TC_IOT_NULL_POINTER);
    IF_EQUAL_RETURN(max_sign_len, 0, TC_IOT_INVALID_PARAMETER);

    ret = tc_iot_calc_sign(
        sha256_digest, sizeof(sha256_digest),
        secret,
        "deviceName=%s&productId=%s",
        device_name, product_id);

    ret = tc_iot_base64_encode((unsigned char *)sha256_digest, sizeof(sha256_digest), b64_buf,
                               sizeof(b64_buf));
    if (ret < sizeof(b64_buf) && ret > 0) {
       b64_buf[ret] = '\0';
       tc_iot_mem_usage_log("b64_buf", sizeof(b64_buf), ret);
    }

    TC_IOT_LOG_TRACE("signature %s", b64_buf);

    url_ret = tc_iot_url_encode(b64_buf, ret, sign_out, max_sign_len);
    if (url_ret < max_sign_len) {
        sign_out[url_ret] = '\0';
    }

    return url_ret;
}

int tc_iot_create_log_form(char* form, int max_form_len,
                                    const char* secret,
                                    const char* device_name,
                                    const char* product_id
                                    ) {
    tc_iot_yabuffer_t form_buf;
    int total = 0;

    IF_NULL_RETURN(form, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(secret, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(device_name, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(product_id, TC_IOT_NULL_POINTER);

    tc_iot_yabuffer_init(&form_buf, form, max_form_len);
    total += tc_iot_add_url_encoded_field(&form_buf, "deviceName=",
                                          device_name, strlen(device_name));
    total += tc_iot_add_url_encoded_field(&form_buf, "&productId=", product_id,
                                          strlen(product_id));
    total += tc_iot_add_url_encoded_field(&form_buf, "&signature=", "", 0);

    total += tc_iot_calc_log_sign(
        tc_iot_yabuffer_current(&form_buf), tc_iot_yabuffer_left(&form_buf),
        secret, device_name, product_id);
    return total;
}

int tc_iot_http_upload_log(tc_iot_device_info* p_device_info, const char * content) {
    char query_string[TC_IOT_HTTP_LOG_REQUEST_FORM_LEN];
    char http_buffer[TC_IOT_HTTP_LOG_RESPONSE_LEN];
    int sign_len;
    int ret;
    char* rsp_body;
    tc_iot_http_client *p_http_client, http_client;

    jsmn_parser p;
    jsmntok_t t[20];

    char temp_buf[TC_IOT_HTTP_MAX_URL_LENGTH];
    int returnCodeIndex = 0;
    int r;
    const int timeout_ms = TC_IOT_API_TIMEOUT_MS;
    bool secured = false;
    uint16_t port = HTTP_DEFAULT_PORT;
#if defined(ENABLE_TLS)
    secured = true;
    port = HTTPS_DEFAULT_PORT;
#endif

    IF_NULL_RETURN(p_device_info, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(content, TC_IOT_NULL_POINTER);

    ret = tc_iot_hal_snprintf(query_string, sizeof(query_string), "%s?", TC_IOT_API_LOG_PATH);
    sign_len = tc_iot_create_log_form(
        query_string+ret, sizeof(query_string)-ret, p_device_info->device_secret,
         p_device_info->device_name,
        p_device_info->product_id);

    tc_iot_mem_usage_log("sign_out[TC_IOT_HTTP_LOG_REQUEST_FORM_LEN]", sizeof(query_string), sign_len);

    TC_IOT_LOG_TRACE("signed request form:\n%s", query_string);

    p_http_client = &http_client;
    tc_iot_http_client_init(p_http_client, HTTP_POST);
    tc_iot_http_client_set_body(p_http_client, content);
    tc_iot_http_client_set_host(p_http_client, p_device_info->log_server_host);
    tc_iot_http_client_set_abs_path(p_http_client, query_string);
    tc_iot_http_client_set_content_type(p_http_client, HTTP_CONTENT_TEXT_PLAIN);

    tc_iot_http_client_format_buffer(http_buffer, sizeof(http_buffer), p_http_client);

    TC_IOT_LOG_TRACE(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    TC_IOT_LOG_TRACE("http_buffer:\n%s", http_buffer);
    TC_IOT_LOG_TRACE("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
    ret = tc_iot_http_client_perform(http_buffer,strlen(http_buffer), sizeof(http_buffer),
                                     p_device_info->log_server_host, port, secured, timeout_ms);
    if (ret == TC_IOT_NET_UNKNOWN_HOST) {
        tc_iot_hal_get_config(TC_IOT_DCFG_LOG_SERVER_IP, temp_buf, sizeof(temp_buf), NULL);
        TC_IOT_LOG_ERROR("host=%s dns lookup failed, try using default ip=%s.", p_device_info->http_host,temp_buf);
        ret = tc_iot_http_client_perform(http_buffer,strlen(http_buffer), sizeof(http_buffer),
                temp_buf, port, secured, timeout_ms);
    }
    tc_iot_mem_usage_log("http_buffer[TC_IOT_HTTP_TOKEN_RESPONSE_LEN]", sizeof(http_buffer), strlen(http_buffer));

    if (ret < 0) {
        TC_IOT_LOG_TRACE("upload log ret=%d", ret);
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
            return TC_IOT_REFRESH_TOKEN_FAILED;
        }

        return TC_IOT_SUCCESS;
    } else {
        return TC_IOT_ERROR_HTTP_REQUEST_FAILED;
    }
}
