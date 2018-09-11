#include "tc_iot_inc.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

long tc_iot_hal_timestamp(void* zone) {
    return time(NULL);
}

int tc_iot_hal_sleep_ms(long sleep_ms) { return usleep(sleep_ms * 1000); }

long tc_iot_hal_random() {
    return random();
}

void tc_iot_hal_srandom(unsigned int seed) { return srand(seed); }

static tc_iot_device_config_data _g_tc_iot_device_config_data = {
    "product_id",
    "product_key",
    "device_name",
    "device_secret",
    "mqtt_host",
    "api_host",
    "region",
};

static int _tc_iot_save_device_config(const char * name, const tc_iot_device_config_data * data) {
    int fd;
    int ret;
    char file_path[64];

    tc_iot_hal_snprintf(file_path, sizeof(file_path), "%s", name);

    fd = open( file_path , O_CREAT|O_WRONLY|O_TRUNC);

    if(fd < 0)
    {
        TC_IOT_LOG_ERROR("create file %s failed.", file_path);
        return TC_IOT_FAILURE;
    }

    ret = write(fd, data, sizeof(*data));
    TC_IOT_LOG_TRACE("ret=%d, data_size=%d", ret, (int)sizeof(*data));
    close(fd);
    return TC_IOT_SUCCESS;
}

static int _tc_iot_load_device_config(const char * name, tc_iot_device_config_data * data) {
    int fd;
    int ret;
    char file_path[64];

    tc_iot_hal_snprintf(file_path, sizeof(file_path), "%s", name);

    fd = open(file_path, O_RDONLY, 0);

    if(fd < 0)
    {
        TC_IOT_LOG_TRACE("key file %s not found.", file_path);
        return TC_IOT_FAILURE;
    }

    ret = read(fd, data, sizeof(*data));
    TC_IOT_LOG_TRACE("ret=%d, data_size=%d", ret, (int)sizeof(*data));
    close(fd);
    return TC_IOT_SUCCESS;
}


int tc_iot_hal_set_value(tc_iot_device_config_id_def id,  const char* value )
{
    tc_iot_device_config_data * p_device_cfg;

    p_device_cfg = &_g_tc_iot_device_config_data;

    switch(id) {
    case TC_IOT_DCFG_PRODUCT_ID:
        strncpy(p_device_cfg->product_id, value, sizeof(p_device_cfg->product_id));
        break;
    case TC_IOT_DCFG_PRODUCT_KEY:
        strncpy(p_device_cfg->product_key, value, sizeof(p_device_cfg->product_key));
        break;
    case TC_IOT_DCFG_DEVICE_NAME:
        strncpy(p_device_cfg->device_name, value, sizeof(p_device_cfg->device_name));
        break;
    case TC_IOT_DCFG_DEVICE_SECRET:
        strncpy(p_device_cfg->device_secret, value, sizeof(p_device_cfg->device_secret));
        break;
    case TC_IOT_DCFG_MQTT_HOST:
        strncpy(p_device_cfg->mqtt_host, value, sizeof(p_device_cfg->mqtt_host));
        break;
    case TC_IOT_DCFG_API_HOST:
        strncpy(p_device_cfg->api_host, value, sizeof(p_device_cfg->api_host));
        break;
    case TC_IOT_DCFG_REGION:
        strncpy(p_device_cfg->region, value, sizeof(p_device_cfg->region));
        break;
    default:
        TC_IOT_LOG_ERROR("config id=%d, value=%s not processed.", id, value);
        break;
    }

    return _tc_iot_save_device_config("device.bin", &_g_tc_iot_device_config_data);
}


char * tc_iot_hal_get_value(tc_iot_device_config_id_def id, char* value , int len, const char * default_var)
{
    int ret = 0;
    tc_iot_device_config_data * p_device_cfg;
    p_device_cfg = &_g_tc_iot_device_config_data;

    ret = _tc_iot_load_device_config("device.bin", p_device_cfg);
    if (ret == TC_IOT_SUCCESS) {

        switch(id) {
        case TC_IOT_DCFG_PRODUCT_ID:
            strncpy( value, p_device_cfg->product_id, sizeof(p_device_cfg->product_id));
            break;
        case TC_IOT_DCFG_PRODUCT_KEY:
            strncpy( value, p_device_cfg->product_key, sizeof(p_device_cfg->product_key));
            break;
        case TC_IOT_DCFG_DEVICE_NAME:
            strncpy( value, p_device_cfg->device_name, sizeof(p_device_cfg->device_name));
            break;
        case TC_IOT_DCFG_DEVICE_SECRET:
            strncpy( value, p_device_cfg->device_secret, sizeof(p_device_cfg->device_secret));
            break;
        case TC_IOT_DCFG_MQTT_HOST:
            strncpy( value, p_device_cfg->mqtt_host, sizeof(p_device_cfg->mqtt_host));
            break;
        case TC_IOT_DCFG_API_HOST:
            strncpy( value, p_device_cfg->api_host, sizeof(p_device_cfg->api_host));
            break;
        case TC_IOT_DCFG_REGION:
            strncpy( value, p_device_cfg->region, sizeof(p_device_cfg->region));
            break;
        default:
            TC_IOT_LOG_ERROR("config id=%d, value=%s not processed.", id, value);
            break;
        }
    } else {
        if (default_var) {
            strncpy(value, default_var, len);
        }
    }
    return value;
}

