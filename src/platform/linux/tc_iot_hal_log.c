#include "tc_iot_inc.h"

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
    }

    if (!g_tc_iot_log_file) {
        g_tc_iot_log_file = fopen(g_tc_iot_log_name, "w+");
        if (!g_tc_iot_log_file) {
            tc_iot_hal_printf("failed to open %s \n", g_tc_iot_log_name);
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
