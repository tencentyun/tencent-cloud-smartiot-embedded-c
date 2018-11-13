#include "tc_iot_device_config.h"
#include "tc_iot_device_logic.h"
#include "tc_iot_export.h"


int tc_iot_log_do_check_and_upload_log();
void parse_command(tc_iot_mqtt_client_config * config, int argc, char ** argv) ;
extern tc_iot_shadow_config g_tc_iot_shadow_config;
extern tc_iot_shadow_local_data g_tc_iot_device_local_data;

/* 循环退出标识 */
volatile int stop = 0;
void sig_handler(int sig) {
    if (sig == SIGINT) {
        TC_IOT_LOG_INFO("SIGINT received, going down.");
        stop ++;
    } else if (sig == SIGTERM) {
        TC_IOT_LOG_INFO("SIGTERM received, going down.");
        stop ++;
    } else {
        TC_IOT_LOG_INFO("signal received:%d", sig);
    }
    if (stop >= 3) {
        TC_IOT_LOG_INFO("SIGINT/SIGTERM received over %d times, force shutdown now.", stop);
        exit(0);
    }
}


/**
 * @brief operate_device 操作设备控制开关
 *
 * @param p_device_data 设备状态数据
 */
void operate_device(unsigned char * changed_bits, tc_iot_shadow_local_data * p_device_data) {
    TC_IOT_LOG_INFO("do something for data change.");
}


/**
 * @brief 本函数演示，当设备端状态发生变化时，如何更新设备端数据，并上报给服务端。
 */
void do_sim_data_change(void) {
/*${data_template.generate_sim_data_change()}*/
}

int main(int argc, char** argv) {
    tc_iot_mqtt_client_config * p_client_config;
    bool use_static_token;
    int ret;
    long timestamp = tc_iot_hal_timestamp(NULL);
    tc_iot_hal_srandom(timestamp);
    long nonce = tc_iot_hal_random();
    tc_iot_device_info * p_device;
    int log_level = TC_IOT_LOG_LEVEL_TRACE;

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    setbuf(stdout, NULL);

    p_client_config = &(g_tc_iot_shadow_config.mqtt_client_config);
    p_device = &p_client_config->device_info;

    /* 解析命令行参数 */
    parse_command(p_client_config, argc, argv);

    tc_iot_hal_get_config(TC_IOT_DCFG_HTTP_HOST, p_device->http_host,
                          sizeof(p_device->http_host), NULL);
    tc_iot_hal_get_config(TC_IOT_DCFG_PRODUCT_ID, p_device->product_id,
                          sizeof(p_device->product_id), NULL);
    tc_iot_hal_get_config(TC_IOT_DCFG_PRODUCT_KEY, p_device->product_key,
                          sizeof(p_device->product_key), NULL);
    tc_iot_hal_get_config(TC_IOT_DCFG_MQTT_HOST, p_device->mqtt_host,
                          sizeof(p_device->mqtt_host), NULL);
    tc_iot_hal_get_config(TC_IOT_DCFG_DEVICE_NAME, p_device->device_name,
                          sizeof(p_device->device_name), NULL);
    tc_iot_hal_get_config(TC_IOT_DCFG_DEVICE_SECRET, p_device->device_secret,
                          sizeof(p_device->device_secret), NULL);

    tc_iot_hal_get_config(TC_IOT_DCFG_LOG_SERVER_HOST, p_device->log_server_host,
                          sizeof(p_device->log_server_host), NULL);

    tc_iot_set_is_up_busilog(atoi(tc_iot_hal_get_config(TC_IOT_DCFG_IS_UP_BUSILOG, NULL, 0, "0")));
    log_level = atoi(tc_iot_hal_get_config(TC_IOT_DCFG_LOG_LEVEL, NULL, 0, "0"));

    tc_iot_hal_snprintf(p_device->client_id, sizeof(p_device->client_id),
                        "%s@%s",p_device->product_key,p_device->device_name);

    tc_iot_log_set_busilog_device( p_device);

    /* ret = tc_iot_http_api_query(&p_client_config->device_info); */
    /* if (ret != TC_IOT_SUCCESS) { */
    /*     TC_IOT_LOG_ERROR("query config failed, trouble shooting guide: " "%s#%d", */
    /*                      TC_IOT_TROUBLE_SHOOTING_URL, ret); */
    /*     #<{(| return 0; |)}># */
    /* } */

    /* 根据 product id 和device name 定义，生成发布和订阅的 Topic 名称。 */
    tc_iot_hal_snprintf(g_tc_iot_shadow_config.sub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, TC_IOT_SHADOW_SUB_TOPIC_FMT,
            p_device->product_id,p_device->device_name);
    tc_iot_hal_snprintf(g_tc_iot_shadow_config.pub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, TC_IOT_SHADOW_PUB_TOPIC_FMT,
            p_device->product_id,p_device->device_name);

    /* 判断是否需要获取动态 token */
    use_static_token = strlen(p_device->username) && strlen(p_device->password);

    if (!use_static_token) {
        /* 获取动态 token */
        TC_IOT_LOG_INFO("requesting username and password for mqtt.");
        ret = TC_IOT_AUTH_FUNC( timestamp, nonce, p_device, TC_IOT_TOKEN_MAX_EXPIRE_SECOND);
        if (ret != TC_IOT_SUCCESS) {
            TC_IOT_LOG_ERROR("refresh token failed, trouble shooting guide: " "%s#%d", TC_IOT_TROUBLE_SHOOTING_URL, ret);
            return 0;
        }
        TC_IOT_LOG_INFO("request username and password for mqtt success.");
    } else {
        TC_IOT_LOG_INFO("username & password using: %s %s", p_device->username, p_device->password);
    }

    ret = tc_iot_data_template_init(tc_iot_get_shadow_client(), &g_tc_iot_shadow_config);
    if (ret != TC_IOT_SUCCESS) {
        if (ret == TC_IOT_NET_UNKNOWN_HOST) {
            TC_IOT_LOG_WARN("tc_iot_data_template_init failed for solve host=%s", p_device->mqtt_host);
            tc_iot_hal_get_config(TC_IOT_DCFG_MQTT_IP, p_device->mqtt_host, sizeof(p_device->mqtt_host), NULL);
            TC_IOT_LOG_WARN("retrying with ip=%s", p_device->mqtt_host);
            ret = tc_iot_data_template_init(tc_iot_get_shadow_client(), &g_tc_iot_shadow_config);
        }
        
        if (ret != TC_IOT_SUCCESS) {
            TC_IOT_LOG_ERROR("tc_iot_data_template_init failed, trouble shooting guide: " "%s#%d", TC_IOT_TROUBLE_SHOOTING_URL, ret);
            return 0;
        }
    }

    ret = tc_iot_data_template_sync(tc_iot_get_shadow_client());
    if (ret != TC_IOT_SUCCESS) {
        TC_IOT_LOG_ERROR("tc_iot_data_template_sync failed, trouble shooting guide: " "%s#%d", TC_IOT_TROUBLE_SHOOTING_URL, ret);
        return 0;
    }

    ret = tc_iot_update_firm_info(tc_iot_get_shadow_client());
    if (ret != TC_IOT_SUCCESS) {
        TC_IOT_LOG_ERROR("tc_iot_update_firm_info failed, trouble shooting guide: " "%s#%d", TC_IOT_TROUBLE_SHOOTING_URL, ret);
        return 0;
    }

    while (!stop) {
        tc_iot_data_template_loop(tc_iot_get_shadow_client(), 200);
        tc_iot_log_do_check_and_upload_log();
    }

    tc_iot_data_template_destroy(tc_iot_get_shadow_client());
    return 0;
}

