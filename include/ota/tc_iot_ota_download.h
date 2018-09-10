#ifndef TC_IOT_OTA_DOWNLOAD
#define TC_IOT_OTA_DOWNLOAD

typedef int (*tc_iot_http_download_callback)(const void * context, const char * data, int data_len, int offset, int total);


int tc_iot_ota_download(const char* api_url, int partial_start, tc_iot_http_download_callback download_callback, const void * context);
int tc_iot_ota_request_content_length(const char* api_url);

#endif /* end of include guard */
