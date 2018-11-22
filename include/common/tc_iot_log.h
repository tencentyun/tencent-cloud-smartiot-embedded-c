#ifndef TC_IOT_LOG_H
#define TC_IOT_LOG_H

#include "tc_iot_inc.h"


/**
 * @brief 日志等级  TRACE < DEBUG < INFO < WARN < ERROR < FATAL < OFF
 */
typedef enum _tc_iot_log_level_e{
    TC_IOT_LOG_LEVEL_TRACE = 0, /**< 输出各类详细过程及输入输出信息 */
    TC_IOT_LOG_LEVEL_DEBUG = 1, /**< 输出应用调试信息 */
    TC_IOT_LOG_LEVEL_INFO = 2,  /**< 输出从粗粒度上，描述了应用运行过程 */
    TC_IOT_LOG_LEVEL_WARN = 3,  /**< 输出潜在的有害状况，需要及时关注 */
    TC_IOT_LOG_LEVEL_ERROR = 4, /**< 输出错误事件，但应用可能还能继续运行 */
    TC_IOT_LOG_LEVEL_FATAL = 5, /**< 输出非常严重的错误事件，可能会导致应用终止执行 */
    TC_IOT_LOG_LEVEL_OFF  = 6,  /**< 关闭所有日志*/
} tc_iot_log_level_e;

void tc_iot_set_log_level(tc_iot_log_level_e log_level);
tc_iot_log_level_e tc_iot_get_log_level(void);
char tc_iot_log_level_enabled(tc_iot_log_level_e log_level);
extern int g_tc_iot_log_is_up_busilog;

int tc_iot_log_set_busilog_device( void * p_device_info);
void * tc_iot_log_get_busilog_device();

int tc_iot_set_is_up_busilog(int is_up);
int tc_iot_get_is_up_busilog();

typedef int (* tc_iot_log_output_handler) (tc_iot_log_level_e level, const char * function, int line, const char * format, ...);

extern const char * g_tc_iot_log_level_str[];

#if !defined(TC_IOT_LOG_NEWLINE)
#define TC_IOT_LOG_NEWLINE "\n"
#endif

#if !defined(TC_IOT_LOG_OUTPUT)
#define TC_IOT_LOG_OUTPUT(level,func,line, ...) \
if (tc_iot_log_level_enabled(level)){               \
    tc_iot_hal_printf("%s %s:%d ",g_tc_iot_log_level_str[level], func, line);                     \
       tc_iot_hal_printf(__VA_ARGS__);                            \
       tc_iot_hal_printf(TC_IOT_LOG_NEWLINE);                                   \
 }

#endif

#if !defined(TC_IOT_LOG_OUTPUT_RAW)
#define TC_IOT_LOG_OUTPUT_RAW(level,func,line, ...)                         \
if (tc_iot_log_level_enabled(level)){                               \
    tc_iot_hal_printf(__VA_ARGS__);                                 \
}

#endif

#ifdef ENABLE_TC_IOT_LOG_TRACE
#define TC_IOT_LOG_TRACE(...) TC_IOT_LOG_OUTPUT(TC_IOT_LOG_LEVEL_TRACE,__FUNCTION__, __LINE__, __VA_ARGS__)
#define TC_IOT_LOG_TRACE_RAW(...) TC_IOT_LOG_OUTPUT_RAW(TC_IOT_LOG_LEVEL_TRACE,__FUNCTION__, __LINE__, __VA_ARGS__)

#define TC_IOT_FUNC_ENTRY TC_IOT_LOG_TRACE("TC_IOT_FUNC_ENTRY")
#define TC_IOT_FUNC_EXIT TC_IOT_LOG_TRACE("TC_IOT_FUNC_EXIT")
#define TC_IOT_FUNC_EXIT_RC(x)  \
    if (tc_iot_log_level_enabled(TC_IOT_LOG_LEVEL_TRACE)){                \
        TC_IOT_LOG_TRACE("TC_IOT_FUNC_EXIT Return : %d", x);              \
    }                                                                     \
    return x;                                                             \

#else
#define TC_IOT_LOG_TRACE(...)
#define TC_IOT_FUNC_ENTRY
#define TC_IOT_FUNC_EXIT
#define TC_IOT_FUNC_EXIT_RC(x) \
    { return x; }
#endif

#ifdef ENABLE_TC_IOT_LOG_DEBUG
#define TC_IOT_LOG_DEBUG(...) TC_IOT_LOG_OUTPUT(TC_IOT_LOG_LEVEL_DEBUG,__FUNCTION__, __LINE__, __VA_ARGS__)

#else
#define TC_IOT_LOG_DEBUG(...)
#endif

#ifdef ENABLE_TC_IOT_LOG_INFO
#define TC_IOT_LOG_INFO(...) TC_IOT_LOG_OUTPUT(TC_IOT_LOG_LEVEL_INFO,__FUNCTION__, __LINE__, __VA_ARGS__)

#else
#define TC_IOT_LOG_INFO(...)
#endif

#ifdef ENABLE_TC_IOT_LOG_WARN
#define TC_IOT_LOG_WARN(...) TC_IOT_LOG_OUTPUT(TC_IOT_LOG_LEVEL_WARN,__FUNCTION__, __LINE__, __VA_ARGS__)

#else
#define TC_IOT_LOG_WARN(...)
#endif

#ifdef ENABLE_TC_IOT_LOG_ERROR
#define TC_IOT_LOG_ERROR(...) TC_IOT_LOG_OUTPUT(TC_IOT_LOG_LEVEL_ERROR,__FUNCTION__, __LINE__, __VA_ARGS__)

#else
#define TC_IOT_LOG_ERROR(...)
#endif

#ifdef ENABLE_TC_IOT_LOG_FATAL
#define TC_IOT_LOG_FATAL(...) TC_IOT_LOG_OUTPUT(TC_IOT_LOG_LEVEL_FATAL,__FUNCTION__, __LINE__, __VA_ARGS__)

#else
#define TC_IOT_LOG_FATAL(...)
#endif

const char * tc_iot_log_summary_string(const char * src, int src_len);

#if defined(ENABLE_STACK_TRACE_LOG)
#define tc_iot_mem_usage_log(name,total,used) TC_IOT_LOG_TRACE("MEM!%s,total=%d,used=%d,left=%d", name, (int)total, (int)used, (int)(total-used))
#else
#define tc_iot_mem_usage_log(name,total,used)
#endif

#endif /* end of include guard */
