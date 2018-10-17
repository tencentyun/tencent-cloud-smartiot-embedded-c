#include "tc_iot_device_config.h"
#include "tc_iot_device_logic.h"
#include "tc_iot_export.h"


struct test_event
{
    uint32_t  timestamp;
    int       user_id;
    char      message[64];
};

struct test_event  local_event;



tc_iot_shadow_property_def  local_event_def1[] = {
    { "timestamp", 1 , &local_event.timestamp } ,
};



EVENT_OBJ_DEF_BEGIN(local_event_def)
    EVENT_OBJ_FIELD_DEF(local_event, TC_IOT_SHADOW_TYPE_TIMESTAMP , timestamp)
    EVENT_OBJ_FIELD_DEF(local_event, TC_IOT_SHADOW_TYPE_INT , user_id)
    EVENT_OBJ_FIELD_DEF(local_event, TC_IOT_SHADOW_TYPE_STRING , message)
EVENT_OBJ_DEF_END



void test_report_single_event()
{
    
    local_event.timestamp = time(NULL);
    local_event.user_id = 3;
    strncpy(local_event.message, "hello world", sizeof(local_event.message));
    tc_iot_report_event_obj (tc_iot_get_shadow_client(), "test_event", TC_IOT_MQTT_METHOD_POST_EVENT,   EVENT_NUM_DEF(local_event_def) , local_event_def);
}

int   int_error = 1;
char  str_error[16] = "error" ;

struct int_error{
    int error ;
}

void test_report_multi_error()
{
    EVENT_LIST_DEF_BEGIN( alarms )
    EVENT_SINGLE_DEF( TC_IOT_SHADOW_TYPE_INT, int_error)
    EVENT_SINGLE_DEF( TC_IOT_SHADOW_TYPE_STRING, str_error)
    EVENT_LIST_DEF_END

    tc_iot_report_multi_event_error(tc_iot_get_shadow_client(), TC_IOT_MQTT_METHOD_RAISE_ERROR, EVENT_NUM_DEF(alarms) , alarms  );
    
}


void test_clear_error()
{
    tc_iot_clear_error(tc_iot_get_shadow_client(), "int_error");
}

void do_sim_report_event_and_error()
{
    printf("~~~~~~~~~~~ report event ~~~~~~~~~~~\n");
    test_report_single_event();

    printf("~~~~~~~~~~~ report error / alarm ~~~~~~~~~~~\n");
    test_report_multi_error();
    test_clear_error();
    printf("~~~~~~~~~~~~~~~~~ report SINGLE error / alarm ~~~~~~~~~~~~~~~~\n");
    int_error = 31;
    REPORT_SINGLE_ERROR( tc_iot_get_shadow_client(), TC_IOT_SHADOW_TYPE_INT, int_error );

    printf("~~~~~~~~~~~~~~~ report event RAW ~~~~~~~~~~~~~~~~~~\n");

    tc_iot_report_event_raw(tc_iot_get_shadow_client(), "{\"OpenDoor\":[{\"unlock_ts\":1535363868,\"type\":1,\"user\":3},{\"unlock_ts\":1535364966,\"type\":2,\"user\":255}]}");

}