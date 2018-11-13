#ifndef TC_IOT_SHADOW_H
#define TC_IOT_SHADOW_H

#define TC_IOT_MAX_PROPERTY_COUNT   128
#define TC_IOT_MAX_FIRM_INFO_COUNT  5

#define TC_IOT_SHADOW_JSON_TRUE   "1"
#define TC_IOT_SHADOW_JSON_FALSE   "0"

#define TC_IOT_SHADOW_SEQUENCE_FIELD  "sequence"

typedef double tc_iot_shadow_number;
typedef int tc_iot_shadow_int;
typedef int tc_iot_shadow_enum;
typedef char tc_iot_shadow_bool;
typedef char * tc_iot_shadow_string;
typedef uint64_t tc_iot_shadow_timestamp;

typedef enum _tc_iot_shadow_data_type_e {
    TC_IOT_SHADOW_TYPE_INVALID = 0,
    TC_IOT_SHADOW_TYPE_BOOL = 1,
    TC_IOT_SHADOW_TYPE_NUMBER = 2,
    TC_IOT_SHADOW_TYPE_ENUM = 3,
    TC_IOT_SHADOW_TYPE_INT = 4,
    TC_IOT_SHADOW_TYPE_STRING = 5,
    TC_IOT_SHADOW_TYPE_TIMESTAMP = 6,

    TC_IOT_SHADOW_TYPE_RAW = 10,
    TC_IOT_SHADOW_TYPE_OBJECT = 11,
    TC_IOT_SHADOW_TYPE_ARRAY = 12,
} tc_iot_shadow_data_type_e;

typedef struct _tc_iot_shadow_property_def {
    const char * name;
    tc_iot_shadow_data_type_e  type;
    void * value;
} tc_iot_shadow_property_def;

typedef struct _tc_iot_shadow_property_meta {
    const char * name;
    tc_iot_shadow_data_type_e  type;
    int id;
    void * value;
} tc_iot_shadow_property_meta;


/**
 * @brief 影子设备配置
 */
typedef struct _tc_iot_shadow_config {
    tc_iot_mqtt_client_config mqtt_client_config;  /**< MQTT 相关配置*/
    char sub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN]; /**< 影子设备订阅 Topic*/
    char pub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN];  /**< 影子设备消息 Publish Topic*/
    int  property_count;
    tc_iot_shadow_property_meta * property_metas;
} tc_iot_shadow_config;

typedef enum _tc_iot_command_ack_status_e {
    TC_IOT_ACK_SUCCESS,
    TC_IOT_ACK_TIMEOUT,
} tc_iot_command_ack_status_e;

typedef void (*message_ack_handler)(tc_iot_command_ack_status_e ack_status, tc_iot_message_data * md , void * session_context);


#define TC_IOT_SESSION_ID_LEN     8
#define TC_IOT_MAX_SESSION_COUNT  10

typedef struct _tc_iot_shadow_session{
    char sid[TC_IOT_SESSION_ID_LEN+1];
    tc_iot_timer        timer;
    message_ack_handler handler;
    void * session_context;
}tc_iot_shadow_session;


/**
 * @brief 影子设备客户端
 */
typedef struct _tc_iot_shadow_client {
    tc_iot_shadow_config* p_shadow_config; /**< 影子设备配置*/
    tc_iot_mqtt_client mqtt_client; /**< MQTT 客户端*/
    tc_iot_shadow_session sessions[TC_IOT_MAX_SESSION_COUNT];
    unsigned int shadow_seq;
} tc_iot_shadow_client;


/*--- begin 影子设备请求响应包 method 字段取值----*/
/* 请求类 */
/**< 读取服务端影子设备数据
 * {"method":"get"}
 * */
#define TC_IOT_MQTT_METHOD_GET       "get"
/**< 更新服务端影子设备数据
 * {"method":"update","state"{"reported":{"a":1}}}
 *
 * */
#define TC_IOT_MQTT_METHOD_UPDATE    "update"

/**< 删除服务端影子设备数据
 * {"method":"delete","state"{"desired":{"a":null}}}
 *
 * */
#define TC_IOT_MQTT_METHOD_DELETE    "delete"

/**< 更新设备状态控制指令
 * {"method":"control","state"{"reported":{"a":1}, "desired":{"a":2}}}
 * */
#define TC_IOT_MQTT_METHOD_CONTROL   "control"

/**< 上报设备信息指令
 * {"method":"update_firm_info","state"{"reported":{"type":"wifi","hw_id":"0011223344"}}}
 * */
#define TC_IOT_MQTT_METHOD_UPDATE_FIRM_INFO   "update_firm_info"

/**< 服务端远程控制指令
 * {"method":"remote_conf","state"{"reboot":0,"log_level":0,"is_up_busilog":0}}
 * */
#define TC_IOT_MQTT_METHOD_REMOTE_CONF   "remote_conf"

/* 响应类 */
/**< 读取请求响应*/
#define TC_IOT_MQTT_METHOD_REPLY     "reply"


/*--- end 影子设备请求响应包 method 字段取值----*/

/**
 * @brief tc_iot_shadow_construct 构造设备影子对象
 *
 * @param p_shadow_client 设备影子对象
 * @param p_config 初始化设备影子对象所需参数配置
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_shadow_construct(tc_iot_shadow_client * p_shadow_client,
                            tc_iot_shadow_config *p_config);


/**
 * @brief tc_iot_shadow_destroy 关闭 Shadow client 连接，并销毁 Shadow client
 *
 * @param p_shadow_client 设备影子对象
 */
void tc_iot_shadow_destroy(tc_iot_shadow_client *p_shadow_client);


/**
 * @brief tc_iot_shadow_isconnected 判断设备影子对象，是否已成功连接服务器
 *
 * @param p_shadow_client 设备影子对象
 *
 * @return 1 表示已连接，0 表示未连接。
 */
char tc_iot_shadow_isconnected(tc_iot_shadow_client *p_shadow_client);

/**
 * @brief tc_iot_shadow_yield 在当前线程为底层服务，让出一定 CPU 执
 * 行时间
 *
 * @param  p_shadow_client 设备影子对象
 * @param timeout_ms 等待时延，单位毫秒
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_shadow_yield(tc_iot_shadow_client *p_shadow_client, int timeout_ms);


/**
 * @brief tc_iot_shadow_get 异步方式获取设备影子文档
 *
 * @param c 设备影子对象
 * @param buffer 设备影子文档缓存
 * @param buffer_len 设备影子文档缓存最大长度
 * @param callback 请求响应数据回调，可选，传 NULL 则表示不指定响应回调，未指定
 * 回调时，服务端响应，则由 shadow 的默认回调函数 on_receive_msg 处理。
 * @param timeout_ms 请求最大等待时延，可选，当指定 callback 参数时，需指定该回调最大
 * 等待时间。
 * @param session_context 请求相关 context，可选，无需透传时，可传默认的 NULL 。
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_shadow_get(tc_iot_shadow_client *c, char * buffer, int buffer_len,
         message_ack_handler callback, int timeout_ms, void * context);

/**
 * @brief tc_iot_shadow_update 异步方式更新设备影子文档
 *
 * @param c 设备影子对象
 * @param buffer 设备影子文档缓存
 * @param buffer_len 设备影子文档缓存最大长度
 * @param reported reported 字段上报数据，可传递三类数据：
 * 1. NULL : 当不需要上报 reported 字段时，传 NULL 。
 * 2. TC_IOT_JSON_NULL : 当需要清空 reported 字段数据时，传 TC_IOT_JSON_NULL 。
 * 3. {"a":1,"b":"some string"} : 当需要正常上报 reported 数据时，传有效的 json 字符串。
 *
 * @param desired desired 字段上报数据，可传递三类数据：
 * 1. NULL : 当不需要上报 desired 字段时，传 NULL 。
 * 2. TC_IOT_JSON_NULL : 当需要清空 desired 字段数据时，传 TC_IOT_JSON_NULL 。
 * 3. {"a":1,"b":"some string"} : 当需要正常上报 desired 数据时，传有效的 json 字符串。
 *
 * @param callback 请求响应数据回调，可选，传 NULL 则表示不指定响应回调，未指定
 * 回调时，服务端响应，则由 shadow 的默认回调函数 on_receive_msg 处理。
 * @param timeout_ms 请求最大等待时延，可选，当指定 callback 参数时，需指定该回调最大
 * 等待时间。
 * @param session_context 请求相关 context，可选，无需透传时，可传默认的 NULL 。
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_shadow_update(tc_iot_shadow_client *c, char * buffer, int buffer_len,
        const char * reported, const char * desired,
        message_ack_handler callback, int timeout_ms, void * session_context);


/**
 * @brief tc_iot_shadow_delete 删除设备属性。
 *
 * @param c 设备影子对象
 * @param buffer 设备影子文档缓存
 * @param buffer_len 设备影子文档缓存最大长度
 * @param reported reported 字段上报数据，可传递三类数据：
 * 1. NULL : 当不需要上报 reported 字段时，传 NULL 。
 * 2. TC_IOT_JSON_NULL : 当需要清空 reported 字段数据时，传 TC_IOT_JSON_NULL 。
 * 3. {"a":1,"b":"some string"} : 当需要正常上报 reported 数据时，传有效的 json 字符串。
 *
 * @param desired desired 字段上报数据，可传递三类数据：
 * 1. NULL : 当不需要上报 desired 字段时，传 NULL 。
 * 2. TC_IOT_JSON_NULL : 当需要清空 desired 字段数据时，传 TC_IOT_JSON_NULL 。
 * 3. {"a":1,"b":"some string"} : 当需要正常上报 desired 数据时，传有效的 json 字符串。
 *
 * @param callback 请求响应数据回调，可选，传 NULL 则表示不指定响应回调，未指定
 * 回调时，服务端响应，则由 shadow 的默认回调函数 on_receive_msg 处理。
 * @param timeout_ms 请求最大等待时延，可选，当指定 callback 参数时，需指定该回调最大
 * 等待时间。
 * @param session_context 请求相关 context，可选，无需透传时，可传默认的 NULL 。
 *
 *
 * @return 结果返回码
 * @see tc_iot_sys_code_e
 */
int tc_iot_shadow_delete(tc_iot_shadow_client *c, char * buffer, int buffer_len,
        const char * reported, const char * desired,
        message_ack_handler callback, int timeout_ms, void * session_context);
int tc_iot_shadow_doc_pack_for_get_with_sid(char *buffer, int buffer_len,
                                    char * session_id, int session_id_len,
                                    bool metadata, bool reported,
                                    tc_iot_shadow_client *c) ;
tc_iot_shadow_session * tc_iot_find_empty_session(tc_iot_shadow_client *c);
tc_iot_shadow_session * tc_iot_fetch_session(tc_iot_shadow_client *c);
void tc_iot_release_session(tc_iot_shadow_session * p_session);
void tc_iot_device_on_message_received(tc_iot_message_data* md);
int _tc_iot_sync_shadow_property(tc_iot_shadow_client * p_shadow_client, bool reported,
        char * doc_start, jsmntok_t * json_token, int tok_count);

int tc_iot_shadow_doc_pack(char *buffer, int buffer_len, const char * method,
                           const char * session_id,
                           const char * reported, const char * desired,
                           tc_iot_shadow_client *c);
int tc_iot_shadow_doc_parse(tc_iot_shadow_client * p_shadow_client,
        char * payload, jsmntok_t * json_token, int token_count, char * field_buf, int field_buf_len);

int tc_iot_shadow_event_notify(tc_iot_shadow_client * p_shadow_client, tc_iot_event_e event, void * data, void * context);
int tc_iot_shadow_report_property(tc_iot_shadow_client * c, int property_id, tc_iot_json_writer * w);
int tc_iot_shadow_check_and_report(tc_iot_shadow_client *c, char * buffer, int buffer_len,
        message_ack_handler callback, int timeout_ms, void * session_context, bool do_confirm);
int tc_iot_shadow_pending_session_count(tc_iot_shadow_client *c);
int _tc_iot_generate_session_id(char * session_id, int session_id_len, tc_iot_mqtt_client* c);

int tc_iot_report_device_data(tc_iot_shadow_client* p_shadow_client, int count, tc_iot_shadow_property_def * p_fields);
int tc_iot_confirm_device_data(tc_iot_shadow_client* p_shadow_client,int count, tc_iot_shadow_property_def * p_fields);
int tc_iot_update_firm_info(tc_iot_shadow_client * c);

int tc_iot_shadow_do_response(tc_iot_shadow_client * p_shadow_client, const char * passthrough);
int tc_iot_shadow_remote_conf_parse(tc_iot_shadow_client * p_shadow_client,
                                    char * payload, jsmntok_t * json_token, int token_count, char * field_buf, int field_buf_len);

int tc_iot_data_template_init(tc_iot_shadow_client* p_shadow_client, tc_iot_shadow_config * p_client_config);
int tc_iot_data_template_sync(tc_iot_shadow_client* p_shadow_client);
int tc_iot_data_template_loop(tc_iot_shadow_client* p_shadow_client, int yield_timeout);
int tc_iot_data_template_destroy(tc_iot_shadow_client* p_shadow_client);

int tc_iot_shadow_up_cmd(tc_iot_shadow_client * c, char * buffer, int buffer_len,
                             const char * method, int count, tc_iot_shadow_property_def * p_fields,
                             message_ack_handler callback, int timeout_ms, void * session_context);

#endif /* end of include guard */

