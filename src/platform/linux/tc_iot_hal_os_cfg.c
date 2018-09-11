
#include "tc_iot_inc.h"

static tc_iot_device_config_data _g_tc_iot_device_config_data = {};

const char * g_tc_iot_device_bin_name = "device.bin";

static int _tc_iot_get_device_config_addr(tc_iot_device_config_data * p_device_cfg, tc_iot_device_config_id_def id, char ** addr, int * len) {

    IF_NULL_RETURN(addr, TC_IOT_NULL_POINTER);
    IF_NULL_RETURN(len, TC_IOT_NULL_POINTER);

    switch(id) {
    case TC_IOT_DCFG_PRODUCT_ID:
        *addr = &p_device_cfg->product_id[0];
        *len = sizeof(p_device_cfg->product_id);
        break;
    case TC_IOT_DCFG_PRODUCT_KEY:
        *addr = &p_device_cfg->product_key[0];
        *len = sizeof(p_device_cfg->product_key);
        break;
    case TC_IOT_DCFG_DEVICE_NAME:
        *addr = &p_device_cfg->device_name[0];
        *len = sizeof(p_device_cfg->device_name);
        break;
    case TC_IOT_DCFG_DEVICE_SECRET:
        *addr = &p_device_cfg->device_secret[0];
        *len = sizeof(p_device_cfg->device_secret);
        break;
    case TC_IOT_DCFG_MQTT_HOST:
        *addr = &p_device_cfg->mqtt_host[0];
        *len = sizeof(p_device_cfg->mqtt_host);
        break;
    case TC_IOT_DCFG_API_HOST:
        *addr = &p_device_cfg->api_host[0];
        *len = sizeof(p_device_cfg->api_host);
        break;
    case TC_IOT_DCFG_REGION:
        *addr = &p_device_cfg->region[0];
        *len = sizeof(p_device_cfg->region);
        break;
    default:
        TC_IOT_LOG_ERROR("config id=%d not processed.", id);
        *addr = NULL;
        *len = 0;
        return TC_IOT_FAILURE;
    }
    return TC_IOT_SUCCESS;
}

static int _tc_iot_save_device_config(const char * name, tc_iot_device_config_data * data) {
    FILE * fp;
    int ret;
    char file_path[64];
    char * addr;
    int len = 0;
    int i = 0;

    tc_iot_hal_snprintf(file_path, sizeof(file_path), "%s", name);

    fp = fopen(name, "w+");

    if(!fp)
    {
        TC_IOT_LOG_ERROR("create file %s failed.", file_path);
        return TC_IOT_FAILURE;
    }

    for (i = 0; i < TC_IOT_DCFG_REGION; i++) {
        ret = _tc_iot_get_device_config_addr(data,i, &addr, &len);
        if (ret == TC_IOT_SUCCESS) {
            ret = fprintf(fp, "%d,%s\n", i, addr);
            if (ret <= 0) {
                TC_IOT_LOG_ERROR("write config failed: %d,%s", i, addr);
                break;
            }
        }
    }
    fclose(fp);
    return TC_IOT_SUCCESS;
}

static int _tc_iot_load_device_config(const char * name, tc_iot_device_config_data * data) {
    FILE * fp;
    int ret;
    char file_path[64];
    char buffer[256];
    int id = 0;
    int len;
    char * addr;

    tc_iot_hal_snprintf(file_path, sizeof(file_path), "%s", name);

    fp = fopen(file_path, "r");

    if(!fp)
    {
        TC_IOT_LOG_ERROR("read file %s failed.", file_path);
        return TC_IOT_FAILURE;
    }

    do {
        ret = fscanf(fp, "%d,%s\n", &id, buffer);
        if (ret < 2) {
            break;
        }
        ret = _tc_iot_get_device_config_addr(data, id, &addr, &len);
        strncpy(addr, buffer, len);
        TC_IOT_LOG_TRACE("loaded: id=%d,value=%s", id, addr);
    } while(true);
    fclose(fp);
    return TC_IOT_SUCCESS;
}

static bool _config_loaded = false;

int tc_iot_hal_set_config(tc_iot_device_config_id_def id,  const char* value )
{
    int ret = 0;
    char  * addr = NULL;
    int len = 0;
    tc_iot_device_config_data * p_device_cfg;

    p_device_cfg = &_g_tc_iot_device_config_data;

    if (!_config_loaded) {
        ret = _tc_iot_load_device_config(g_tc_iot_device_bin_name, p_device_cfg);
        _config_loaded = true;
    }

    ret = _tc_iot_get_device_config_addr(p_device_cfg, id, &addr, &len);
    if (ret == TC_IOT_SUCCESS) {
        if (strcmp(addr, value) == 0) {
            TC_IOT_LOG_TRACE("config id=%d, value=%s equals, skip save.", id, value);
            return TC_IOT_SUCCESS;
        } else {
            strncpy(addr, value, len);
        }
    } else {
        TC_IOT_LOG_ERROR("config id=%d, value=%s not processed.", id, value);
        return TC_IOT_FAILURE;
    }

    TC_IOT_LOG_TRACE("set config id=%d, value=%s success.", id, value);
    return _tc_iot_save_device_config(g_tc_iot_device_bin_name, &_g_tc_iot_device_config_data);
}


char * tc_iot_hal_get_config(tc_iot_device_config_id_def id, char* value , int len, const char * default_var)
{
    int ret = 0;
    tc_iot_device_config_data * p_device_cfg;
    char * addr;
    int src_len;

    p_device_cfg = &_g_tc_iot_device_config_data;

    if (!_config_loaded) {
        ret = _tc_iot_load_device_config(g_tc_iot_device_bin_name, p_device_cfg);
        _config_loaded = true;
    }

    if (ret == TC_IOT_SUCCESS) {
        ret = _tc_iot_get_device_config_addr(p_device_cfg,id, &addr, &src_len);
        if (ret == TC_IOT_SUCCESS) {
            if (strlen(addr) > 0) {
                strncpy(value, addr, len);
                TC_IOT_LOG_TRACE("get config id=%d, value=%s success.", id, value);
                return value;
            } else {
                TC_IOT_LOG_WARN("config id=%d, value=%s is empty.", id, value);
            }
        } else {
            TC_IOT_LOG_ERROR("config id=%d, value=%s not processed.", id, value);
        }
    }

    if (default_var) {
        strncpy(value, default_var, len);
        TC_IOT_LOG_TRACE("get config id=%d, value=%s using default_var.", id, value);
    }
    return value;
}
