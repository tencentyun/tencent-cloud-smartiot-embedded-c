#ifndef TC_IOT_HAL_OS_H
#define TC_IOT_HAL_OS_H

#include "tc_iot_inc.h"


/**
 * @brief tc_iot_hal_malloc 分配所需的内存空间，并返回一个指向它的指针
 *
 * @param size 待分配空间的大小
 *
 * @return 指针指向分配到的内存空间，返回空指针则表示未分配到内存
 */
// void *tc_iot_hal_malloc(size_t size);


/**
 * @brief tc_iot_hal_free 释放之前调用 tc_iot_hal_malloc 所分配的内存空间。 
 *
 * @param ptr 待释放的内存
 */
// void tc_iot_hal_free(void *ptr);


/**
 * @brief tc_iot_hal_printf 发送格式化输出到标准输出 stdout
 *
 * @param format 格式化字符串
 * @param ... 格式化相关输入参数
 *
 * @return 输出字节数
 */
/*int tc_iot_hal_printf(const char *format, ...);*/

/**
 * @brief tc_iot_hal_snprintf 发送格式化输出到字符串
 *
 * @param str 输出缓存区
 * @param size 输出缓存区大小
 * @param format 格式化字符串
 * @param ... 格式化相关输入参数
 *
 * @return 输出字节数，需要注意的是：当返回值与入参size大小一致时，有可能缓存区
 * 大小不够，内容并未完全输出
 */
/* int tc_iot_hal_snprintf(char *str, size_t size, const char *format, ...); */


/**
 * @brief tc_iot_hal_timestamp 系统时间戳，格林威治时间 1970-1-1 00点起总秒数
 *
 * @param 保留参数，暂未启用
 *
 * @return 时间戳
 */
long tc_iot_hal_timestamp(void *);


/**
 * @brief tc_iot_hal_sleep_ms 睡眠挂起一定时长，单位：ms
 *
 * @param sleep_ms 时长，单位ms
 *
 * @return 0 表示成功，-1 表示失败
 */
int tc_iot_hal_sleep_ms(long sleep_ms);


/**
 * @brief tc_iot_hal_random 获取随机数
 *
 * @return 获得的随机数
 */
long tc_iot_hal_random(void);



/**
 * @brief tc_iot_hal_srandom 设置随机数种子值
 *
 * @param seed 随机数种子值
 */
void tc_iot_hal_srandom(unsigned int seed);


/**
 * @brief 设备配置项 ID 定义
 */
typedef enum _tc_iot_device_config_id_def {
    // Required
    TC_IOT_DCFG_PRODUCT_ID,
    TC_IOT_DCFG_PRODUCT_KEY,
    TC_IOT_DCFG_DEVICE_NAME,
    TC_IOT_DCFG_DEVICE_SECRET,
    TC_IOT_DCFG_MQTT_HOST,
    TC_IOT_DCFG_API_HOST,
    TC_IOT_DCFG_REGION,

    TC_IOT_DCFG_TYPE,
    TC_IOT_DCFG_HW_ID,
    TC_IOT_DCFG_MODULE,
    TC_IOT_DCFG_MODULE_VER,
    TC_IOT_DCFG_FIRM_VER,
    TC_IOT_DCFG_LAT,
    TC_IOT_DCFG_LON,
    TC_IOT_DCFG_KEEPALIVE,
    TC_IOT_DCFG_LOG_LEVEL,
    TC_IOT_DCFG_IS_UP_BUSILOG,

    // Optional
    TC_IOT_DCFG_PRODUCT_PASSWORD,
    TC_IOT_DCFG_MQTT_TLS_CA_CERT,
    TC_IOT_DCFG_MQTT_TLS_CLIENT_KEY,
    TC_IOT_DCFG_MQTT_TLS_CLIENT_CERT,
    TC_IOT_DCFG_HTTPS_CA_CERT,
} tc_iot_device_config_id_def;

#define TC_IOT_MAX_MQTT_HOST_LEN  128
#define TC_IOT_MAX_API_HOST_LEN  128
#define TC_IOT_MAX_REGION_LEN  128
#define TC_IOT_MAX_TYPE_LEN   5
#define TC_IOT_MAX_LOG_LEVEL_LEN  2
#define TC_IOT_MAX_KEEP_ALIVE_LEN 4
#define TC_IOT_MAX_UP_BUSILOG_LEN  2


/**
 * @brief 持久化保存一个 id - value 值, 比如保存在文件系统或者 flash 里面
 *
 * @param  id		输入参数, 保存的 id
 * @param  value    输入参数, 保存的value
 *
 * @return  <0  表示失败,  成功返回 0
 */
int tc_iot_hal_set_config(tc_iot_device_config_id_def id,  const char* value );

/**
 * @brief 读取保存的 id - value 值
 *
 * @param  id		输入参数, 保存的 id
 * @param  value    输出参数, value
 * @param  len      value缓存区大小
 * @param  default_var 当 id 指定的数据不存在时，返回此默认数据。
 *
 *
 * @return  value 指针地址
 */
const char * tc_iot_hal_get_config(tc_iot_device_config_id_def id, char* value , int len, const char * default_var);

int tc_iot_log_linux_output_handler(tc_iot_log_level_e level, const char * func, int line, const char * format, ...);
#undef TC_IOT_LOG_OUTPUT
#define TC_IOT_LOG_OUTPUT tc_iot_log_linux_output_handler


#endif /* end of include guard */
