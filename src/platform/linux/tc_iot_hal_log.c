#include "tc_iot_inc.h"

#define TC_IOT_BUSILOG_UPLOAD_SIZE     512

int tc_iot_log_do_check_and_upload_log(const char * log_name);

static FILE * g_tc_iot_log_file = NULL;
static const char * g_tc_iot_log_name = "device.log";

int tc_iot_log_linux_output_handler(tc_iot_log_level_e level, const char * func, int line, const char * format, ...) {
    int ret = 0;
    va_list args;
    time_t timer;
    char time_str[26];
    struct tm* tm_info;
    int log_file_size = 0;
    char log_new_name[64];
    int previous_log_level = 0;

    time(&timer);
    tm_info = localtime(&timer);

    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

    if (tc_iot_log_level_enabled(level)){
        printf("%s %s %s:%d ", time_str, g_tc_iot_log_level_str[level], func, line);
        va_start(args, format);
        vprintf(format, args);
        printf(TC_IOT_LOG_NEWLINE);
    } else {
        return TC_IOT_SUCCESS;
    }

    if (level <= TC_IOT_LOG_LEVEL_TRACE) {
        // TRACE log too much skip log to local.
        return TC_IOT_SUCCESS;
    }

    if (!g_tc_iot_log_file) {
        g_tc_iot_log_file = fopen(g_tc_iot_log_name, "w+");
        if (!g_tc_iot_log_file) {
            tc_iot_hal_printf("ERROR: failed to open %s \n", g_tc_iot_log_name);
            return TC_IOT_FAILURE;
        }
    }

    va_start(args, format);
    fprintf(g_tc_iot_log_file, "%s %s %s:%d ", time_str, g_tc_iot_log_level_str[level], func, line);
    vfprintf(g_tc_iot_log_file, format, args);
    fprintf(g_tc_iot_log_file, TC_IOT_LOG_NEWLINE);
    va_end(args);

    if (tc_iot_get_is_up_busilog()) {
        log_file_size = ftell(g_tc_iot_log_file);
        if (log_file_size >= TC_IOT_BUSILOG_UPLOAD_SIZE) {
            fclose(g_tc_iot_log_file);
            g_tc_iot_log_file = NULL;

            strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H%M%S", tm_info);
            tc_iot_hal_snprintf(log_new_name, sizeof(log_new_name), "%s.%s", g_tc_iot_log_name, time_str);
            ret =  rename (g_tc_iot_log_name, log_new_name);
            if (ret != 0) {
                TC_IOT_LOG_ERROR("rename file %s to %s failed, error=%d",  g_tc_iot_log_name, log_new_name, ret);
                return TC_IOT_SUCCESS;
            }
            /* previous_log_level = tc_iot_get_log_level(); */
            /* tc_iot_set_log_level(TC_IOT_LOG_LEVEL_OFF); */
            tc_iot_log_do_check_and_upload_log(log_new_name);
            /* tc_iot_set_log_level(previous_log_level); */
        }
    }

    return TC_IOT_SUCCESS;
}

int tc_iot_log_do_check_and_upload_log(const char * log_name) {
    char buffer[TC_IOT_BUSILOG_UPLOAD_SIZE*2];
    FILE * log_file = NULL;
    int log_file_size = 0;
    int ret = 0;

    log_file = fopen(log_name, "r+");
    if (!log_file) {
        return TC_IOT_FAILURE;
    }

    /* fseek(log_file, 0L, SEEK_END); */
    /* log_file_size = ftell(log_file); */
    /* if (log_file_size >= sizeof(buffer) || log_file_size <= 0) { */
    /*     tc_iot_hal_printf("ERROR: invalid %s log_file_size=%d > %d \n", log_name, log_file_size, (int)sizeof(buffer)); */
    /*     return TC_IOT_FAILURE; */
    /* } */
    fseek(log_file, 0L, SEEK_SET);
    ret = fread(buffer, 1, sizeof(buffer)-1, log_file);
    if (ret <= 0 ) {
        tc_iot_hal_printf("ERROR: buffer over flow ret=%d\n",ret);
        return TC_IOT_FAILURE;
    }
    buffer[ret] = '\0';
    ret = tc_iot_http_upload_log(tc_iot_log_get_busilog_device(), buffer);
    return ret ;
}
