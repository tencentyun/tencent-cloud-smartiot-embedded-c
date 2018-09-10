#include "tc_iot_inc.h"

unsigned int tc_iot_sntp_get_timestamp(const char * host, int timeout_ms) {
    tc_iot_network_t network;
    tc_iot_network_t* p_network;
    tc_iot_net_context_init_t netcontext;
    int written_len = 0;
    int read_len = 0;
    unsigned int tx_time_s = 0;
    unsigned int ts = 0;

    tc_iot_sntp_packet packet = {
        TC_IOT_SNTP_PACK_LVM(TC_IOT_SNTP_DEFAULT_LI, TC_IOT_SNTP_DEFAULT_VN, TC_IOT_SNTP_DEFAULT_MODE),
        TC_IOT_SNTP_DEFAULT_STRATUM, TC_IOT_SNTP_DEFAULT_POLL, TC_IOT_SNTP_DEFAULT_PREC,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    p_network = &network;
    memset(p_network, 0, sizeof(tc_iot_network_t));

    netcontext.fd = -1;
    netcontext.use_tls = 0;
    netcontext.host = host;
    netcontext.port = TC_IOT_SNTP_DEFAULT_PORT;
    tc_iot_hal_udp_init(p_network, &netcontext);

    p_network->do_connect(p_network, NULL, 0);

    written_len = p_network->do_write(p_network, (unsigned char *)&packet, sizeof(packet), timeout_ms);
    if (written_len != sizeof(packet)) {
        TC_IOT_LOG_ERROR("send sntp request failed: host=%s, written_len=%d, expected=%d", host, written_len, (int)sizeof(packet));
        return 0;
    } else {
        TC_IOT_LOG_TRACE("send sntp udp success: host=%s, written_len=%d", host, written_len);
    }
    read_len = p_network->do_read(p_network, (unsigned char *)&packet, sizeof(packet), timeout_ms);
    if (read_len != sizeof(packet)) {
        TC_IOT_LOG_ERROR("receive sntp response failed: host=%s, read_len=%d, expected=%d", host, read_len, (int)sizeof(packet));
        return 0;
    } else {
        TC_IOT_LOG_TRACE("receive sntp response success: host=%s, read_len=%d", host, read_len);
    }

    tx_time_s = tc_iot_ntohl( packet.tx_time_s );

    // ntp start from 1900, unix timestamp start from 1970, subtract extra 70 years.
    ts = ( unsigned int) ( tx_time_s - TC_IOT_SNTP_TIMESTAMP_DELTA );

    TC_IOT_LOG_TRACE( "current timestamp: %u", ts);

    p_network->do_disconnect(p_network);

    return ts;
}
