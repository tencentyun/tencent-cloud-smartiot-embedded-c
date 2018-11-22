#include "tc_iot_export.h"
#include "tc_iot_device_config.h"
#include <getopt.h>

static int _log_level = TC_IOT_LOG_LEVEL_DEBUG;
static int request_token = 1;
static int show_version = 0;
char g_tc_iot_device_config_name[64] = "device_profile.conf";
int tc_iot_load_device_config(const char * config_name);
int tc_iot_save_device_config(const char * config_name);

static struct option long_options[] =
{
    {"verbose",      no_argument,          &_log_level, TC_IOT_LOG_LEVEL_INFO},
    {"trace",        no_argument,          &_log_level, TC_IOT_LOG_LEVEL_TRACE},
    {"version",        no_argument,        &show_version, 1},
    {"config",       optional_argument,    0, 'c'},
    {"port",         optional_argument,    0, 'p'},
    {"help",         optional_argument,    0, '?'},
    {0, 0, 0, 0}
};

const char * command_help =
"Usage: %s [-c config] [-p port] [--trace|--verbose]\r"
"\r"
"       %s --help\r"
"\r"
" -c config\r"
" --config=config file name\r"
"     Device config file name.\r"
" \r"
" -p port\r"
" --port=port\r"
"     network port to connect to. Defaults to 1883, or set to 8883 for tls is MQTT server supports.\r"
"\r"
" --verbose\r"
"     don't print info and debug, trace messages.\r"
"\r"
" --trace\r"
"     print all system messages, for debug or problem tracing.\r"
"  -?\r"
" --help\r"
"     display this message.\r"
;

void parse_command(tc_iot_mqtt_client_config * config, int argc, char ** argv) {
    int c;
    int option_index = 0;
    bool device_name_changed = false;
    bool client_id_changed = false;
    int left = 0;
    int i = 0;

    while (1)
    {
        c = getopt_long (argc, argv, "c:p:?", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c)
        {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0) {
                    break;
                }
                TC_IOT_LOG_INFO ("option %s", long_options[option_index].name);
                if (optarg) {
                    TC_IOT_LOG_INFO (" with arg %s", optarg);
                }
                TC_IOT_LOG_INFO ("");
                break;
            case 'h':
                if (optarg) {
                    sprintf(config->device_info.mqtt_host,"%s",optarg);
                    TC_IOT_LOG_INFO ("host=%s", optarg);
                }
                break;
            case 'c':
                if (optarg) {
                    strncpy(g_tc_iot_device_config_name, optarg, sizeof(g_tc_iot_device_config_name));
                    TC_IOT_LOG_INFO ("config=%s", optarg);
                }
                break;
            case 'p':
                if (optarg) {
                    config->device_info.mqtt_port =  atoi(optarg);
                    if (config->device_info.mqtt_port == 8883) {
                        config->use_tls = 1;
                        TC_IOT_LOG_INFO ("port=%s", optarg);
                    } else if (config->device_info.mqtt_port == 1883) {
                        config->use_tls = 0;
                        TC_IOT_LOG_INFO ("port=%s", optarg);
                    } else if (config->device_info.mqtt_port == 0){
                        TC_IOT_LOG_INFO ("invalid port=%s", optarg);
                        exit(0);
                    } else {
                        config->use_tls = (config->device_info.mqtt_port > 8883);
                        TC_IOT_LOG_INFO ("WARNING: unknown port=%d", (int)config->device_info.mqtt_port);
                    }
                }
                break;
            case 's':
                if (optarg) {
                    strncpy(config->device_info.device_secret, optarg, sizeof(config->device_info.device_secret));
                    TC_IOT_LOG_INFO ("secret=%s", optarg);
                }
                break;
            case 'u':
                if (optarg) {
                    strncpy(config->device_info.username, optarg, TC_IOT_MAX_USER_NAME_LEN);
                    TC_IOT_LOG_INFO ("username=%s", config->device_info.username);
                    request_token = 0;
                }
                break;
            case 'P':
                if (optarg) {
                    strncpy(config->device_info.password, optarg, TC_IOT_MAX_PASSWORD_LEN);
                    TC_IOT_LOG_INFO ("password=%s", config->device_info.password);
                    request_token = 0;
                }
                break;
            case 't':
                if (optarg) {
                    strncpy(config->device_info.product_id, optarg, TC_IOT_MAX_PRODUCT_ID_LEN);
                    TC_IOT_LOG_INFO ("product id=%s", config->device_info.product_id);
                }
                break;
            case 'd':
                if (optarg) {
                    strncpy(config->device_info.device_name, optarg, TC_IOT_MAX_DEVICE_NAME_LEN);
                    device_name_changed = true;
                    TC_IOT_LOG_INFO ("device name=%s", config->device_info.device_name);
                }
                break;
            case 'i':
                if (optarg) {
                    client_id_changed = true;
                    strncpy(config->device_info.client_id, optarg, TC_IOT_MAX_CLIENT_ID_LEN);
                    TC_IOT_LOG_INFO ("client id=%s", config->device_info.client_id);
                }
                break;
	    case '?':
		TC_IOT_LOG_INFO (command_help, argv[0]);
                exit(0);
		break;

            default:
                TC_IOT_LOG_INFO("option: %c", (char)c);
                break;
        }
    }

    if (device_name_changed && !client_id_changed) {
        for (i = 0; i < TC_IOT_MAX_CLIENT_ID_LEN; i++) {
            if (config->device_info.client_id[i] == '@') {
                left = TC_IOT_MAX_CLIENT_ID_LEN - i- 2;
                strncpy(config->device_info.client_id+i+1, config->device_info.device_name, left);
                config->device_info.client_id[TC_IOT_MAX_CLIENT_ID_LEN-1] = '\0';
                break;
            }
        }
    }

    tc_iot_load_device_config(g_tc_iot_device_config_name);
    tc_iot_set_log_level(_log_level);

    if (optind < argc)
    {
        TC_IOT_LOG_INFO ("non-option ARGV-elements: ");
        while (optind < argc) {
            TC_IOT_LOG_INFO ("%s ", argv[optind++]);
        }
        TC_IOT_LOG_INFO ("");
    }
    if (show_version) {
        TC_IOT_LOG_INFO ("%s based on Tencent Cloud IoT Suite C SDK Version %s", argv[0], TC_IOT_SDK_VERSION);
        exit(0);
    }
}

