#include "tc_iot_inc.h"

#define TC_IOT_BUSILOG_UPLOAD_SIZE     512

int tc_iot_log_do_check_and_upload_log();

static FILE * g_tc_iot_log_file = NULL;
static const char * g_tc_iot_log_name = "device.log";

int tc_iot_log_linux_output_handler(tc_iot_log_level_e level, const char * func, int line, const char * format, ...) {
    va_list args;
    time_t timer;
    char time_str[26];
    struct tm* tm_info;

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


    return TC_IOT_SUCCESS;
}

int tc_iot_log_do_check_and_upload_log() {
    char buffer[TC_IOT_BUSILOG_UPLOAD_SIZE*2];
    FILE * log_file = NULL;
    int log_file_size = 0;
    time_t timer;
    int ret = 0;
    char time_str[26];
    struct tm* tm_info;
    char log_name[64];

    if (!tc_iot_get_is_up_busilog()) {
        return TC_IOT_SUCCESS;
    }

    if (!g_tc_iot_log_file) {
        return TC_IOT_SUCCESS;
    }

    log_file_size = ftell(g_tc_iot_log_file);
    if (log_file_size >= TC_IOT_BUSILOG_UPLOAD_SIZE) {
        fclose(g_tc_iot_log_file);
        g_tc_iot_log_file = NULL;
    }

    time(&timer);
    tm_info = localtime(&timer);

    strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H%M%S", tm_info);
    tc_iot_hal_snprintf(log_name, sizeof(log_name), "%s.%s", g_tc_iot_log_name, time_str);
    ret =  rename (g_tc_iot_log_name, log_name);
    if (ret != 0) {
        TC_IOT_LOG_ERROR("rename file %s to %s failed, error=%d",  g_tc_iot_log_name, log_name, ret);
        return TC_IOT_SUCCESS;
    }

    log_file = fopen(log_name, "r+");
    if (!log_file) {
        return TC_IOT_FAILURE;
    }

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
