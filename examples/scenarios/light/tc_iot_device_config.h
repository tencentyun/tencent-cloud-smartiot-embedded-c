#ifndef TC_IOT_DEVICE_CONFIG_H
#define TC_IOT_DEVICE_CONFIG_H

#include "tc_iot_config.h"

/**********************************必填项区域 begin ********************************/
/* 设备激活及获取 secret 接口，地址格式为：<机房标识>.auth-device-iot.tencentcloudapi.com/secret */
/* Token接口，地址格式为：<机房标识>.auth-device-iot.tencentcloudapi.com/token */
/* 机房标识：
    广州机房=gz
    北京机房=bj
    ...
*/
#define TC_IOT_CONFIG_REGION  "gz"

/* 以下配置需要先在官网创建产品和设备，然后获取相关信息更新*/
/* MQ服务地址，可以在产品“基本信息”->“mqtt链接地址”位置找到。*/
#define TC_IOT_CONFIG_MQTT_HOST "mqtt-1doou8fjk.ap-guangzhou.mqtt.tencentcloudmq.com"

/* 产品id，可以在产品“基本信息”->“产品id”位置找到*/
#define TC_IOT_CONFIG_PRODUCT_ID "iot-9fi4gnz8"

/* 产品id，可以在产品“基本信息”->“产品key”位置找到*/
#define TC_IOT_CONFIG_PRODUCT_KEY "mqtt-1doou8fjk"

/* 设备密钥，可以在产品“设备管理”->“设备证书”->“Device Secret”位置找到*/
#define TC_IOT_CONFIG_DEVICE_SECRET "00000000000000000000000000000000"

/* 设备名称，可以在产品“设备管理”->“设备名称”位置找到*/
#define TC_IOT_CONFIG_DEVICE_NAME "device_name"

/* 鉴权模式，可以在产品的“基本信息”->“鉴权模式”位置找到
 * 1:动态令牌模式
 * 2:签名认证模式
 * */
#define TC_IOT_CONFIG_AUTH_MODE   3

/**********************************必填项区域 end ********************************/


/**********************************选填项区域 begin ********************************/
/*
 * 除非处于调试或特殊应用场景，以下内容一般情况下不需要手动修改。
 * */

/*------------------MQTT begin---------------------*/
#ifdef ENABLE_TLS
// 请求 API 时使用的协议，https 或者 http
#define TC_IOT_CONFIG_API_HTTP_PROTOCOL "https"

// MQ 服务的 MQTT 是否通过 TLS 协议通讯，1 为使用，0 表示不使用
#define TC_IOT_CONFIG_USE_TLS 1 
/* MQ服务的TLS端口一般为8883*/
#define TC_IOT_CONFIG_MQTT_PORT 8883

#else
// 请求 API 时使用的协议，https 或者 http
#define TC_IOT_CONFIG_API_HTTP_PROTOCOL "http"

// MQ 服务的 MQTT 是否通过 TLS 协议通讯，1 为使用，0 表示不使用
#define TC_IOT_CONFIG_USE_TLS 0
/* MQ服务的默认端口一般为1883*/
#define TC_IOT_CONFIG_MQTT_PORT 1883
#endif

/* client id 由两部分组成，组成形式为“ProductKey@DeviceName” */
#define TC_IOT_CONFIG_DEVICE_CLIENT_ID TC_IOT_CONFIG_PRODUCT_KEY "@" TC_IOT_CONFIG_DEVICE_NAME


/* 关于username和password：*/
/* 1)如果是通过TC_IOT_CONFIG_AUTH_API_URL接口，动态获取的，以下两个参数可不用填写*/
/* 2)如果有预先申请好的固定username和password，可以把获取到的固定参数填写到如下位置*/
#if TC_IOT_CONFIG_AUTH_MODE == 1
/* 动态令牌模式 */
#define TC_IOT_CONFIG_DEVICE_USER_NAME ""
#define TC_IOT_CONFIG_DEVICE_PASSWORD ""
#define TC_IOT_AUTH_FUNC   tc_iot_refresh_auth_token
#elif TC_IOT_CONFIG_AUTH_MODE == 2
/*签名认证模式*/
#define TC_IOT_AUTH_FUNC   tc_iot_mqtt_refresh_dynamic_sign
#define TC_IOT_CONFIG_DEVICE_USER_NAME ""
#define TC_IOT_CONFIG_DEVICE_PASSWORD ""
#elif TC_IOT_CONFIG_AUTH_MODE == 3
/*iothub认证模式*/
#define TC_IOT_AUTH_FUNC   tc_iot_mqtt_refresh_iothub_dynamic_sign
#define TC_IOT_CONFIG_DEVICE_USER_NAME ""
#define TC_IOT_CONFIG_DEVICE_PASSWORD ""
#else
/* 直连模式 */
#define TC_IOT_AUTH_FUNC(a,b,c,d)   
#define TC_IOT_CONFIG_DEVICE_USER_NAME ""
#define TC_IOT_CONFIG_DEVICE_PASSWORD ""
#endif

// begin: DEBUG use only 
#define TC_IOT_CONFIG_ACTIVE_API_URL_DEBUG   "http://" TC_IOT_CONFIG_REGION ".auth.iot.cloud.tencent.com/secret"
#define TC_IOT_CONFIG_AUTH_API_URL_DEBUG	 "http://" TC_IOT_CONFIG_REGION ".auth.iot.cloud.tencent.com/token"
// end: DEBUG use only 

// API 服务域名根地址
#define TC_IOT_CONFIG_API_HOST TC_IOT_CONFIG_REGION ".token.smartiot.cloud.tencent.com"

/* shadow下行消息topic，mq服务端的响应和下行推送，*/
/* 都会发布到 "shadow/get/<product id>/<device name>" 这个topic*/
/* 客户端只需要订阅这个topic即可*/
#define TC_IOT_SHADOW_SUB_TOPIC_PREFIX "shadow/get/"
#define TC_IOT_SHADOW_SUB_TOPIC_FMT TC_IOT_SHADOW_SUB_TOPIC_PREFIX "%s/%s"
#define TC_IOT_SHADOW_SUB_TOPIC_DEF TC_IOT_SHADOW_SUB_TOPIC_PREFIX TC_IOT_CONFIG_PRODUCT_ID "/" TC_IOT_CONFIG_DEVICE_NAME

/* shadow上行消息topic，客户端请求服务端的消息，发到到这个topic即可*/
/* topic格式"shadow/update/<product id>/<device name>"*/
#define TC_IOT_SHADOW_PUB_TOPIC_PREFIX "shadow/update/"
#define TC_IOT_SHADOW_PUB_TOPIC_FMT TC_IOT_SHADOW_PUB_TOPIC_PREFIX "%s/%s"
#define TC_IOT_SHADOW_PUB_TOPIC_DEF TC_IOT_SHADOW_PUB_TOPIC_PREFIX TC_IOT_CONFIG_PRODUCT_ID "/" TC_IOT_CONFIG_DEVICE_NAME

/*------------------MQTT end---------------------*/

/**********************************选填项区域 end ********************************/

#endif /* end of include guard */
