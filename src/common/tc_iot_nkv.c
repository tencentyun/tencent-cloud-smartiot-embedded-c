#include "tc_iot_inc.h"


tc_iot_nkv_item * tc_iot_nkv_item_header(tc_iot_nkv_container * c) {
    if (!c) {
        return NULL;
    }
    return c->header;
}

tc_iot_nkv_item * tc_iot_nkv_item_find(tc_iot_nkv_container * c, const char * name) {
    int i = 0;
    tc_iot_nkv_item * data = tc_iot_nkv_item_header(c);

    if (!data) {
        TC_IOT_LOG_ERROR("data is null");
        return NULL;
    }

    for (i = 0; i < c->len; ++i,++data) {
        if ((data->type == NKV_INVALID) || (strcmp(data->name, name) == 0)) {
            return data;
        }
    }

    return NULL;
}

int tc_iot_nkv_iterm_clear(tc_iot_nkv_item * header, int len) {
    IF_NULL_RETURN(header, TC_IOT_NULL_POINTER);
    memset(header, 0, sizeof(*header)*len );
    return TC_IOT_SUCCESS;
}

int tc_iot_nkv_init(tc_iot_nkv_container * c,  tc_iot_nkv_item * header, int len) {

    IF_NULL_RETURN(c, TC_IOT_NULL_POINTER);

    c->header = header;
    c->len = len;

    return TC_IOT_SUCCESS;
}


int tc_iot_nkv_set_string(tc_iot_nkv_container * c,const char * name, const char * value) {
    tc_iot_nkv_item * data = tc_iot_nkv_item_find(c, name);
    if (data) {
        data->type = NKV_STRING;
        if (!data->name) {
            data->name = name;
        }
        data->d.sv = value;
        return TC_IOT_SUCCESS;
    }

    return TC_IOT_NKV_OVERFLOW;

}


const char * tc_iot_nkv_get_string(tc_iot_nkv_container * c, const char * name, const char * default_str) {
    tc_iot_nkv_item * data = tc_iot_nkv_item_find(c, name);
    if (data && (data->type != NKV_INVALID)) {
        return data->d.sv;
    }

    return default_str;
}

int tc_iot_nkv_get_int32(tc_iot_nkv_container * c, const char * name, int default_int) {
    tc_iot_nkv_item * data = tc_iot_nkv_item_find(c, name);
    if (data && (data->type != NKV_INVALID)) {
        return data->d.iv;
    }

    return default_int;
}

int tc_iot_nkv_set_int32(tc_iot_nkv_container * c,const char * name, int value) {
    tc_iot_nkv_item * data = tc_iot_nkv_item_find(c, name);
    if (data) {
        data->type = NKV_INT32;
        if (!data->name) {
            data->name = name;
        }
        data->d.iv = value;
        return TC_IOT_SUCCESS;
    }

    return TC_IOT_NKV_OVERFLOW;
}

#if defined(ENABLE_NKV_DOUBLE)
int tc_iot_nkv_set_double(tc_iot_nkv_container * c,const char * name, double value) {
    tc_iot_nkv_item * data = tc_iot_nkv_item_find(c, name);
    if (data) {
        data->type = NKV_DOUBLE;
        if (!data->name) {
            data->name = name;
        }
        data->d.dv = value;
        return TC_IOT_SUCCESS;
    }

    return TC_IOT_NKV_OVERFLOW;
}

double tc_iot_nkv_get_double(tc_iot_nkv_container * c, const char * name, double default_double) {
    tc_iot_nkv_item * data = tc_iot_nkv_item_find(c, name);
    if (data && (data->type != NKV_INVALID)) {
        return data->d.dv;
    }

    return default_double;
}

#endif

