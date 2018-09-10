#ifndef IOT_HTTP_UTILS_H
#define IOT_HTTP_UTILS_H

#include "tc_iot_inc.h"

#define HTTP_VERSION_1_0 "1.0"
#define HTTP_VERSION_1_1 "1.1"

#define HTTP_PUT "PUT"
#define HTTP_HEAD "HEAD"
#define HTTP_POST "POST"
#define HTTP_GET "GET"

#define TC_IOT_USER_AGENT "tciotclient/1.0"

#define HTTP_SPLIT_STR "\r\n"
#define HTTP_REQUEST_LINE_FMT ("%s %s HTTP/%s" HTTP_SPLIT_STR)
#define HTTP_HEADER_FMT "%s: %s\r\n"
#define HTTP_HEADER_HOST "Host"
#define HTTP_HEADER_ACCEPT "Accept"
#define HTTP_HEADER_ACCEPT_ENCODING "Accept-Encoding"
#define HTTP_HEADER_USER_AGENT "User-Agent"
#define HTTP_HEADER_CONTENT_LENGTH "Content-Length"
#define HTTP_HEADER_LOCATION "Location"
#define HTTP_HEADER_CONTENT_TYPE "Content-Type"
#define HTTP_CONTENT_FORM_URLENCODED "application/x-www-form-urlencoded"
#define HTTP_CONTENT_OCTET_STREAM "application/octet-stream"
#define HTTP_CONTENT_FORM_DATA "multipart/form-data"
#define HTTP_CONTENT_JSON "application/json"

#define HTTPS_PREFIX "https"
#define HTTPS_PREFIX_LEN (sizeof(HTTPS_PREFIX) - 1)

#define TC_IOT_HTTP_MAX_URL_LENGTH     128
#define TC_IOT_HTTP_MAX_HOST_LENGTH    128


/* examples: */
/* HTTP/1.0 200 OK */
/* HTTP/1.1 404 Not Found */
/* HTTP/1.1 503 Service Unavailable */
#define HTTP_RESPONSE_STATE_PREFIX "HTTP/1."
#define HTTP_RESPONSE_STATE_PREFIX_LEN (sizeof(HTTP_RESPONSE_STATE_PREFIX)-1)


#define HTTP_BODY_FMT "\r\n%s"

/**
 * @brief tc_iot_calc_auth_sign 计算 Token 请求签名
 *
 * @param sign_out 签名( Base64 编码)结果
 * @param max_sign_len 签名( Bsse64 编码)结果区长度
 * @param secret 签名密钥
 * @param client_id Client Id
 * @param device_name Device Name
 * @param expire Token有效期
 * @param nonce 随机数
 * @param product_id Product Id
 * @param timestamp 时间戳
 *
 * @return >=0 签名结果实际长度，<0 错误码
 * @see tc_iot_sys_code_e
 */
int tc_iot_calc_auth_sign(char* sign_out, int max_sign_len, const char* secret,
                          const char* client_id,
                          const char* device_name,
                          long expire, long nonce,
                          const char* product_id, 
                          long timestamp);


/**
 * @brief tc_iot_create_auth_request_form 构造 Token HTTP 签名请求 form
 *
 * @param form 结果缓存区
 * @param max_form_len 结果缓存区最大大小
 * @param secret 签名密钥
 * @param client_id Client Id
 * @param device_name Device Name
 * @param expire Token有效期
 * @param nonce 随机数
 * @param product_id Product Id
 * @param timestamp 时间戳
 *
 * @return >=0 签名结果实际长度，<0 错误码
 * @see tc_iot_sys_code_e
 */
int tc_iot_create_auth_request_form(char* form,  int max_form_len,
                                    const char* secret, 
                                    const char* client_id, 
                                    const char* device_name,
                                    long expire,
                                    long nonce,
                                    const char* product_id,
                                    long timestamp);

/**
 * @brief tc_iot_create_active_device_form 构造 get device 设备激活 HTTP 签名请求 form
 *
 * @param form 结果缓存区
 * @param max_form_len 结果缓存区最大大小
 * @param secret 签名密钥, 请使用控制台的 product password
 * @param device_name Device Name
 * @param product_id Product Id , 例子 : "iot-dalqbv1g"	
 * @param nonce 随机数
 * @param timestamp 时间戳 *
 * @return >=0 签名结果实际长度，<0 错误码
 * @see tc_iot_sys_code_e
 */
int tc_iot_create_active_device_form(char* form, int max_form_len,
									const char* product_secret, 
                                    const char* device_name,  
									const char* product_id,
                                    long nonce, long timestamp);

/**
 * @brief tc_iot_parse_http_response_code 解析 HTTP 响应数据返回码
 *
 * @param resp HTTP 响应数据字符串
 *
 * @return >0 HTTP Status Code, 比如 200、404、50x 等， <0 数据格式非法，无法解
 * 析
 */
int tc_iot_parse_http_response_code(const char * http_resp);

typedef int (*tc_iot_http_response_callback)(const void * context, const char * data, int data_len, int offset, int total);

typedef enum _tc_iot_http_response_parse_state {
    _PARSER_START,
    _PARSER_VERSION,
    _PARSER_IGNORE_TO_RETURN_CHAR,
    _PARSER_SKIP_NEWLINE_CHAR,
    _PARSER_HEADER,
    _PARSER_IGNORE_TO_BODY_START,
    _PARSER_END,
} tc_iot_http_response_parse_state;

typedef struct _tc_iot_http_response_parser {
    tc_iot_http_response_parse_state state;
    char  version;
    short status_code;
    int  content_length;
    const char * location;
    // const char * body;
}tc_iot_http_response_parser;

void tc_iot_http_parser_init(tc_iot_http_response_parser * parser);
int tc_iot_http_parser_analysis(tc_iot_http_response_parser * parser, const char * buffer, int buffer_len);

typedef struct _tc_iot_http_client {
    const char * method; // http method: get, post, put, head ...
    const char * version; //  http version: 1.0, 1.1 ...
    const char * host; // request host: www.example.com
    const char * content_type;
    const char * abs_path; // url absolute path: /absolute/path
    const char * extra_headers; // extra headers: Range, Content-Type ...

    const char * body; // http request body
} tc_iot_http_client;

int tc_iot_http_client_init(tc_iot_http_client * c, const char * method);
int tc_iot_http_client_set_version(tc_iot_http_client * c, const char * version);
int tc_iot_http_client_set_host(tc_iot_http_client * c, const char * host);
int tc_iot_http_client_set_abs_path(tc_iot_http_client * c, const char * abs_path);
int tc_iot_http_client_set_content_type(tc_iot_http_client * c, const char * content_type);
int tc_iot_http_client_set_extra_headers(tc_iot_http_client * c, const char * extra_headers);
int tc_iot_http_client_set_body(tc_iot_http_client * c, const char * body);

int tc_iot_http_client_format_buffer(char * buffer, int buffer_len, tc_iot_http_client * c);
int tc_iot_http_client_internal_perform(char * buffer, int buffer_used, int buffer_len,
                                        tc_iot_network_t * p_network,tc_iot_http_response_parser * p_parser,
                                        const char * host, uint16_t port,
                                        bool secured, int timeout_ms, tc_iot_http_response_callback resp_callback, const void * callback_context);
int tc_iot_http_client_perform(char * buffer, int buffer_used, int buffer_len,
                               const char * host, uint16_t port,
                               bool secured, int timeout_ms);


#endif /* end of include guard */
