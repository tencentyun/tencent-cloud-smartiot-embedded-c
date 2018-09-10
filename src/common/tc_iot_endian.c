#include "tc_iot_inc.h"

const int g_tc_iot_endianness_flag = 1;

uint32_t tc_iot_htonl(uint32_t a) {
    if (TC_IOT_IS_BIG_ENDIAN()) {
        return a;
    } else {
        return TC_IOT_UINT32_ENDIAN_SWAP(a);
    }
}

uint32_t tc_iot_ntohl(uint32_t a) {
    if (TC_IOT_IS_BIG_ENDIAN()) {
        return a;
    } else {
        return TC_IOT_UINT32_ENDIAN_SWAP(a);
    }
}

uint16_t tc_iot_htons(uint16_t a) {
    if (TC_IOT_IS_BIG_ENDIAN()) {
        return a;
    } else {
        return TC_IOT_UINT16_ENDIAN_SWAP(a);
    }
}

uint16_t tc_iot_ntohs(uint16_t a) {
    if (TC_IOT_IS_BIG_ENDIAN()) {
        return a;
    } else {
        return TC_IOT_UINT16_ENDIAN_SWAP(a);
    }
}

