#ifndef TC_IOT_NKV_H
#define TC_IOT_NKV_H

typedef enum _TC_IOT_NKV_TYPE {
    NKV_INVALID,
    NKV_INT32,
    NKV_STRING,
#if defined(ENABLE_NKV_DOUBLE)
    NKV_DOUBLE,
#endif
    NKV_BUTT = 0xFF,
} TC_IOT_NKV_TYPE;

typedef struct _tc_iot_nkv_item {
    const char * name;
    TC_IOT_NKV_TYPE type;
    union _data {
        int iv;
        double dv;
        const char * sv;
    } d;
} tc_iot_nkv_item;


typedef struct _tc_iot_nkv_container {
    int len;
    tc_iot_nkv_item * header;
} tc_iot_nkv_container;

tc_iot_nkv_item * tc_iot_nkv_item_header(tc_iot_nkv_container * c);
tc_iot_nkv_item * tc_iot_nkv_item_find(tc_iot_nkv_container * c, const char * name);
int tc_iot_nkv_iterm_clear(tc_iot_nkv_item * header, int len);

int tc_iot_nkv_init(tc_iot_nkv_container * c,  tc_iot_nkv_item * header, int len);

int tc_iot_nkv_set_string(tc_iot_nkv_container * c, const char * name, const char * value);
const char * tc_iot_nkv_get_string(tc_iot_nkv_container * c, const char * name, const char * default_str);

int tc_iot_nkv_set_int32(tc_iot_nkv_container * c, const char * name, int value);
int tc_iot_nkv_get_int32(tc_iot_nkv_container * c, const char * name, int default_int);


#if defined(ENABLE_NKV_DOUBLE)
int tc_iot_nkv_set_double(tc_iot_nkv_container * c, const char * name, double value);
double tc_iot_nkv_get_double(tc_iot_nkv_container * c, const char * name, double default_double);
#endif

#endif
