#ifndef TC_IOT_ENDIAN_H
#define TC_IOT_ENDIAN_H

extern const int g_tc_iot_endianness_flag;

#define TC_IOT_IS_BIG_ENDIAN() ( (*(char*)&g_tc_iot_endianness_flag) == 0 )
#define TC_IOT_IS_LITTLE_ENDIAN() ( (*(char*)&g_tc_iot_endianness_flag) == 1 )

#define TC_IOT_UINT32_ENDIAN_SWAP(a)   ((((uint32_t)(a) & 0xFF000000) >> 24) |  \
                                        (((uint32_t)(a) & 0x00FF0000) >> 8) |   \
                                        (((uint32_t)(a) & 0x0000FF00) << 8) |   \
                                        (((uint32_t)(a) & 0x000000FF) << 24))

#define TC_IOT_UINT16_ENDIAN_SWAP(a) (((uint16_t)((a) & 0xFF00) >> 8) | ((uint16_t)((a) & 0xFF) << 8))

uint32_t tc_iot_htonl(uint32_t a);
uint32_t tc_iot_ntohl(uint32_t a);

uint16_t tc_iot_htons(uint16_t a);
uint16_t tc_iot_ntohs(uint16_t a);

#endif /* TC_IOT_ENDIAN_H */
