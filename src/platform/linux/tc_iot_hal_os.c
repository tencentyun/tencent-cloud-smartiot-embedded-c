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

static const char * g_tc_iot_dcfg_id_mapping[] = {
    "dcfg_product_id",
    "dcfg_product_key",
    "dcfg_region",
    "dcfg_device_name",
    "dcfg_device_secret",

    "dcfg_product_password",
};

int tc_iot_hal_set_value(tc_iot_device_config_id_def id,  const char* value )
{

    int fd;

    char file_path[64];

    tc_iot_hal_snprintf(file_path, sizeof(file_path), "%s", g_tc_iot_dcfg_id_mapping[id]);

    fd = open( file_path , O_CREAT|O_WRONLY|O_TRUNC);

    if(fd < 0)
    {
        TC_IOT_LOG_ERROR("create file %s failed.", file_path);
        return TC_IOT_FAILURE;
    }

    write(fd, value, strlen(value) + 1);
    close(fd);
    return 0;
}


char * tc_iot_hal_get_value(tc_iot_device_config_id_def id, char* value , int len , const char * default_var)
{
    int fd;

    char file_path[64];

    tc_iot_hal_snprintf(file_path, sizeof(file_path), "%s", g_tc_iot_dcfg_id_mapping[id]);

    fd = open(file_path, O_RDONLY, 0);

    if(fd < 0)
    {
        if (default_var) {
            tc_iot_hal_snprintf(value, len, "%s", default_var);
        }
        TC_IOT_LOG_TRACE("key file %s for %d not found.", file_path, id);
        return value;
    }

    read(fd, value, len);
    close(fd);
    return value;
}

