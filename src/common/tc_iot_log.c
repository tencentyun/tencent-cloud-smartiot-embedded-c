#include "tc_iot_inc.h"

static char g_tc_iot_log_summary_print_str[64];
const char * g_tc_iot_log_level_str[] = {
    "TRACE", "DEBUG", "INFO",
    "WARN", "ERROR", "FATAL",
    "",
};

static tc_iot_log_level_e g_tc_iot_log_level = TC_IOT_LOG_LEVEL_TRACE;

int g_tc_iot_log_is_up_busilog = 0;
static void * g_tc_iot_log_device = NULL;

void tc_iot_set_log_level(tc_iot_log_level_e log_level) {
    g_tc_iot_log_level = log_level;
}

tc_iot_log_level_e tc_iot_get_log_level(void) {
    return g_tc_iot_log_level ;
}

char tc_iot_log_level_enabled(tc_iot_log_level_e log_level) {
    return log_level >= g_tc_iot_log_level;
}

const char * tc_iot_log_summary_string(const char * src, int src_len) {
    int print_buf_len = sizeof(g_tc_iot_log_summary_print_str);

    if (src_len >= print_buf_len) {
        strncpy(g_tc_iot_log_summary_print_str, src, print_buf_len);
        strcpy(&g_tc_iot_log_summary_print_str[0]+print_buf_len-5, "...");
    } else {
        strncpy(&g_tc_iot_log_summary_print_str[0], src, src_len);
        g_tc_iot_log_summary_print_str[src_len] = '\0';
    }

    return &g_tc_iot_log_summary_print_str[0];
}

int tc_iot_log_set_busilog_device( void * p_device_info) {
    g_tc_iot_log_device = p_device_info;
    return TC_IOT_SUCCESS;
}

void * tc_iot_log_get_busilog_device() {
    return g_tc_iot_log_device;
}

int tc_iot_set_is_up_busilog(int is_up) {
    g_tc_iot_log_is_up_busilog = is_up;
    return g_tc_iot_log_is_up_busilog;
}

int tc_iot_get_is_up_busilog() {
    return g_tc_iot_log_is_up_busilog;
}
