#include "tc_iot_inc.h"

typedef struct _tc_iot_device_config_data {
    char product_id[TC_IOT_MAX_PRODUCT_ID_LEN];
    char product_key[TC_IOT_MAX_PRODUCT_KEY_LEN];
    char device_name[TC_IOT_MAX_DEVICE_NAME_LEN];
    char device_secret[TC_IOT_MAX_DEVICE_SECRET_LEN];
    char mqtt_host[TC_IOT_MAX_HOST_LEN];
    char mqtt_ip[TC_IOT_MAX_IP_LEN];
    char http_host[TC_IOT_MAX_HOST_LEN];
    char http_ip[TC_IOT_MAX_IP_LEN];
    char log_server_host[TC_IOT_MAX_HOST_LEN];
    char log_server_ip[TC_IOT_MAX_IP_LEN];
    char region[TC_IOT_MAX_REGION_LEN];

    char type[TC_IOT_MAX_TYPE_LEN];
    char hw_id[20];
    char module[20];
    char module_ver[20];
    char firm_ver[20];
    char lat[20];
    char lon[20];
    char keepalive[TC_IOT_MAX_KEEP_ALIVE_LEN];
    char log_level[TC_IOT_MAX_LOG_LEVEL_LEN];
    char is_up_busilog[TC_IOT_MAX_UP_BUSILOG_LEN];
}tc_iot_device_config_data;

static tc_iot_device_config_data _g_tc_iot_device_config_data = {};

const char * g_tc_iot_device_bin_name = "device_config.bin";

tc_iot_code_map _g_tc_iot_device_config_id_map[] = {
    {TC_IOT_DCFG_PRODUCT_ID,"product_id"},
    {TC_IOT_DCFG_PRODUCT_KEY,"product_key"},
    {TC_IOT_DCFG_DEVICE_NAME,"device_name"},
    {TC_IOT_DCFG_DEVICE_SECRET,"device_secret"},
    {TC_IOT_DCFG_MQTT_HOST,"mqtt_host"},
    {TC_IOT_DCFG_MQTT_IP,"mqtt_ip"},
    {TC_IOT_DCFG_HTTP_HOST,"http_host"},
    {TC_IOT_DCFG_HTTP_IP,"http_ip"},
    {TC_IOT_DCFG_LOG_SERVER_HOST,"log_server_host"},
    {TC_IOT_DCFG_LOG_SERVER_IP,"log_server_ip"},
    {TC_IOT_DCFG_REGION,"region"},
    {TC_IOT_DCFG_TYPE,"type"},
    {TC_IOT_DCFG_HW_ID,"hw_id"},
    {TC_IOT_DCFG_MODULE,"module"},
    {TC_IOT_DCFG_MODULE_VER,"module_ver"},
    {TC_IOT_DCFG_FIRM_VER,"firm_ver"},
    {TC_IOT_DCFG_LAT,"lat"},
    {TC_IOT_DCFG_LON,"lon"},
    {TC_IOT_DCFG_KEEPALIVE,"keepalive"},
    {TC_IOT_DCFG_LOG_LEVEL,"log_level"},
    {TC_IOT_DCFG_IS_UP_BUSILOG,"is_up_busilog"},
};

const char * tc_iot_get_device_config_name_by_id(tc_iot_device_config_id_def id) {
    int i = 0;
    tc_iot_code_map * map = NULL;

    for (i = 0; i < TC_IOT_ARRAY_LENGTH(_g_tc_iot_device_config_id_map); ++i) {
        map = &_g_tc_iot_device_config_id_map[i];
        if (map->code == id) {
            return map->str;
        }
    }

    return "";
}

int tc_iot_get_device_config_id_by_name(const char * name) {
    int i = 0;
    tc_iot_code_map * map = NULL;

    for (i = 0; i < TC_IOT_ARRAY_LENGTH(_g_tc_iot_device_config_id_map); ++i) {
        map = &_g_tc_iot_device_config_id_map[i];
        if (strcmp(map->str, name) == 0) {
            return map->code;
        }
    }

    return -1;
}

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
    case TC_IOT_DCFG_MQTT_IP:
        *addr = &p_device_cfg->mqtt_ip[0];
        *len = sizeof(p_device_cfg->mqtt_ip);
        break;
    case TC_IOT_DCFG_HTTP_HOST:
        *addr = &p_device_cfg->http_host[0];
        *len = sizeof(p_device_cfg->http_host);
        break;
    case TC_IOT_DCFG_HTTP_IP:
        *addr = &p_device_cfg->http_ip[0];
        *len = sizeof(p_device_cfg->http_ip);
        break;
    case TC_IOT_DCFG_LOG_SERVER_HOST:
        *addr = &p_device_cfg->log_server_host[0];
        *len = sizeof(p_device_cfg->log_server_host);
        break;
    case TC_IOT_DCFG_LOG_SERVER_IP:
        *addr = &p_device_cfg->log_server_ip[0];
        *len = sizeof(p_device_cfg->log_server_ip);
        break;
    case TC_IOT_DCFG_REGION:
        *addr = &p_device_cfg->region[0];
        *len = sizeof(p_device_cfg->region);
        break;
    case TC_IOT_DCFG_TYPE:
        *addr = &p_device_cfg->type[0];
        *len = sizeof(p_device_cfg->type);
        break;
    case TC_IOT_DCFG_HW_ID:
        *addr = &p_device_cfg->hw_id[0];
        *len = sizeof(p_device_cfg->hw_id);
        break;
    case TC_IOT_DCFG_MODULE:
        *addr = &p_device_cfg->module[0];
        *len = sizeof(p_device_cfg->module);
        break;
    case TC_IOT_DCFG_MODULE_VER:
        *addr = &p_device_cfg->module_ver[0];
        *len = sizeof(p_device_cfg->module_ver);
        break;
    case TC_IOT_DCFG_FIRM_VER:
        *addr = &p_device_cfg->firm_ver[0];
        *len = sizeof(p_device_cfg->firm_ver);
        break;
    case TC_IOT_DCFG_LAT:
        *addr = &p_device_cfg->lat[0];
        *len = sizeof(p_device_cfg->lat);
        break;
    case TC_IOT_DCFG_LON:
        *addr = &p_device_cfg->lon[0];
        *len = sizeof(p_device_cfg->lon);
        break;
    case TC_IOT_DCFG_KEEPALIVE:
        *addr = &p_device_cfg->keepalive[0];
        *len = sizeof(p_device_cfg->keepalive);
        break;
    case TC_IOT_DCFG_LOG_LEVEL:
        *addr = &p_device_cfg->log_level[0];
        *len = sizeof(p_device_cfg->log_level);
        break;
    case TC_IOT_DCFG_IS_UP_BUSILOG:
        *addr = &p_device_cfg->is_up_busilog[0];
        *len = sizeof(p_device_cfg->is_up_busilog);
        break;
    default:
        TC_IOT_LOG_ERROR("config id=%d not processed.", id);
        *addr = NULL;
        *len = 0;
        return TC_IOT_FAILURE;
    }
    return TC_IOT_SUCCESS;
}

int tc_iot_save_device_config(const char * config_name) {
    FILE * fp;
    int ret;
    char file_path[64];
    char * addr;
    int len = 0;
    int i = 0;
    tc_iot_device_config_data * data = &_g_tc_iot_device_config_data;

    tc_iot_hal_snprintf(file_path, sizeof(file_path), "%s", config_name);

    fp = fopen(file_path, "w+");

    if(!fp)
    {
        TC_IOT_LOG_ERROR("create file %s failed.", file_path);
        return TC_IOT_FAILURE;
    }

    for (i = 0; i < TC_IOT_DCFG_TOTAL; i++) {
        ret = _tc_iot_get_device_config_addr(data,i, &addr, &len);
        if (ret == TC_IOT_SUCCESS) {
            if (strlen(addr) > 0) {
                ret = fprintf(fp, "%s,%s\n", tc_iot_get_device_config_name_by_id(i), addr);
                if (ret <= 0) {
                    TC_IOT_LOG_ERROR("write config failed: %d,%s", i, addr);
                    break;
                }
            } else {
                TC_IOT_LOG_TRACE("skip empty value for id=%d", i);
            }
        }
    }
    fclose(fp);
    return TC_IOT_SUCCESS;
}

int tc_iot_load_device_config(const char * config_name) {
    FILE * fp;
    int ret;
    char file_path[64];
    char buffer[256];
    int id = 0;
    int len;
    char * addr;
    char name_buf[128];
    tc_iot_device_config_data * data = &_g_tc_iot_device_config_data;

    tc_iot_hal_snprintf(file_path, sizeof(file_path), "%s", config_name);

    fp = fopen(file_path, "r");

    if(!fp)
    {
        TC_IOT_LOG_ERROR("read file %s failed.", file_path);
        return TC_IOT_FAILURE;
    }

    do {
        ret = fscanf(fp, "%[^=]=%s\n", name_buf, buffer);
        if (ret < 2) {
            if (ret != -1) {
                TC_IOT_LOG_ERROR("ret=%d, config format invalid.", ret);
            }
            break;
        }
        id = tc_iot_get_device_config_id_by_name(name_buf);
        if (id < 0) {
            if (name_buf[0] != '#') {
                TC_IOT_LOG_WARN("%s unrecorgnizable for return id=%d", name_buf, id);
            }
            continue;
        }
        ret = _tc_iot_get_device_config_addr(data, id, &addr, &len);
        strncpy(addr, buffer, len);
        TC_IOT_LOG_TRACE("loaded: name=%s,id=%d,value=%s", name_buf, id, addr);
    } while(true);
    fclose(fp);
    return TC_IOT_SUCCESS;
}


int tc_iot_hal_set_config(tc_iot_device_config_id_def id,  const char* value )
{
    int ret = 0;
    char  * addr = NULL;
    int len = 0;
    tc_iot_device_config_data * p_device_cfg;

    p_device_cfg = &_g_tc_iot_device_config_data;

    ret = _tc_iot_get_device_config_addr(p_device_cfg, id, &addr, &len);
    if (ret == TC_IOT_SUCCESS) {
        if (strcmp(addr, value) == 0) {
            TC_IOT_LOG_TRACE("config name=%s, id=%d, value=%s equals, skip save.",
                             tc_iot_get_device_config_name_by_id(id), id, value);
            return TC_IOT_SUCCESS;
        } else {
            strncpy(addr, value, len);
        }
    } else {
        TC_IOT_LOG_ERROR("config id=%d, value=%s not processed.", id, value);
        return TC_IOT_FAILURE;
    }

    TC_IOT_LOG_TRACE("set config name=%s, id=%d, value=%s success.", tc_iot_get_device_config_name_by_id(id), id, value);
    return TC_IOT_SUCCESS;
}


const char * tc_iot_hal_get_config(tc_iot_device_config_id_def id, char* value , int len, const char * default_var)
{
    int ret = 0;
    tc_iot_device_config_data * p_device_cfg;
    char * addr;
    int src_len;

    p_device_cfg = &_g_tc_iot_device_config_data;

    if (ret == TC_IOT_SUCCESS) {
        ret = _tc_iot_get_device_config_addr(p_device_cfg,id, &addr, &src_len);
        if (ret == TC_IOT_SUCCESS) {
            if (strlen(addr) > 0) {
                if (value) {
                    strncpy(value, addr, len);
                    TC_IOT_LOG_TRACE("get config name=%s, id=%d, value=%s success.",
                                     tc_iot_get_device_config_name_by_id(id), id, value);
                    return value;
                } else {
                    return addr;
                }
            /* } else { */
            /*     TC_IOT_LOG_WARN("config id=%d, value=%s is empty.", id, addr); */
            }
        } else {
            TC_IOT_LOG_ERROR("config id=%d, value=%s not processed.", id, value);
        }
    }

    if (default_var) {
        if (value) {
            strncpy(value, default_var, len);
            TC_IOT_LOG_TRACE("get config id=%d, value=%s using default_var.", id, value);
        } else {
            return default_var;
        }
    }
    return value;
}
