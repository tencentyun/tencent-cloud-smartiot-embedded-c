#ifndef TC_IOT_SNTP_H
#define TC_IOT_SNTP_H
#include "tc_iot_inc.h"

/*
  see: https://tools.ietf.org/html/rfc2030

  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |LI | VN  |Mode |    Stratum    |     Poll      |   Precision   |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                          Root Delay                           |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                       Root Dispersion                         |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                     Reference Identifier                      |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                                                               |
  |                   Reference Timestamp (64)                    |
  |                                                               |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                                                               |
  |                   Originate Timestamp (64)                    |
  |                                                               |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                                                               |
  |                    Receive Timestamp (64)                     |
  |                                                               |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                                                               |
  |                    Transmit Timestamp (64)                    |
  |                                                               |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                 Key Identifier (optional) (32)                |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                                                               |
  |                                                               |
  |                 Message Digest (optional) (128)               |
  |                                                               |
  |                                                               |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

  Leap Indicator (LI): This is a two-bit code warning of an impending
  leap second to be inserted/deleted in the last minute of the current
  day, with bit 0 and bit 1, respectively, coded as follows:

  LI       Value     Meaning
  -------------------------------------------------------
  00       0         no warning
  01       1         last minute has 61 seconds
  10       2         last minute has 59 seconds)
  11       3         alarm condition (clock not synchronized)

  Version Number (VN): This is a three-bit integer indicating the
  NTP/SNTP version number. The version number is 3 for Version 3 (IPv4
  only) and 4 for Version 4 (IPv4, IPv6 and OSI). If necessary to
  distinguish between IPv4, IPv6 and OSI, the encapsulating context
  must be inspected.

  Mode: This is a three-bit integer indicating the mode, with values
  defined as follows:

  Mode     Meaning
  ------------------------------------
  0        reserved
  1        symmetric active
  2        symmetric passive
  3        client
  4        server
  5        broadcast
  6        reserved for NTP control message
  7        reserved for private use

  In unicast and anycast modes, the client sets this field to 3
  (client) in the request and the server sets it to 4 (server) in the
  reply. In multicast mode, the server sets this field to 5
  (broadcast).

  Stratum: This is a eight-bit unsigned integer indicating the stratum
  level of the local clock, with values defined as follows:

  Stratum  Meaning
  ----------------------------------------------
  0        unspecified or unavailable
  1        primary reference (e.g., radio clock)
  2-15     secondary reference (via NTP or SNTP)
  16-255   reserved

  Poll Interval: This is an eight-bit signed integer indicating the
  maximum interval between successive messages, in seconds to the
  nearest power of two. The values that can appear in this field
  presently range from 4 (16 s) to 14 (16284 s); however, most
  applications use only the sub-range 6 (64 s) to 10 (1024 s).

  Precision: This is an eight-bit signed integer indicating the
  precision of the local clock, in seconds to the nearest power of two.
  The values that normally appear in this field range from -6 for
  mains-frequency clocks to -20 for microsecond clocks found in some
  workstations.

  Root Delay: This is a 32-bit signed fixed-point number indicating the
  total roundtrip delay to the primary reference source, in seconds
  with fraction point between bits 15 and 16. Note that this variable
  can take on both positive and negative values, depending on the
  relative time and frequency offsets. The values that normally appear
  in this field range from negative values of a few milliseconds to
  positive values of several hundred milliseconds.

  Root Dispersion: This is a 32-bit unsigned fixed-point number
  indicating the nominal error relative to the primary reference
  source, in seconds with fraction point between bits 15 and 16. The
  values that normally appear in this field range from 0 to several
  hundred milliseconds.

  Reference Identifier: This is a 32-bit bitstring identifying the
  particular reference source. In the case of NTP Version 3 or Version
  4 stratum-0 (unspecified) or stratum-1 (primary) servers, this is a
  four-character ASCII string, left justified and zero padded to 32
  bits. In NTP Version 3 secondary servers, this is the 32-bit IPv4
  address of the reference source. In NTP Version 4 secondary servers,
  this is the low order 32 bits of the latest transmit timestamp of the
  reference source. NTP primary (stratum 1) servers should set this
  field to a code identifying the external reference source according
  to the following list. If the external reference is one of those
  listed, the associated code should be used. Codes for sources not
  listed can be contrived as appropriate.

*/

#define TC_IOT_SNTP_TIMESTAMP_DELTA 2208988800ull

#define TC_IOT_SNTP_GET_LI(packet)   (((packet).li_vn_mode >> 6) & 0x3)
#define TC_IOT_SNTP_GET_VN(packet)   (((packet).li_vn_mode >> 3) & 0x7)
#define TC_IOT_SNTP_GET_MODE(packet) ((packet).li_vn_mode        & 0x7)

#define TC_IOT_SNTP_PACK_LVM(li,vn,mode) (((li) << 6) | ((vn) << 3) | (mode))

#define TC_IOT_SNTP_DEFAULT_PORT        		 (123)

// no warning
#define TC_IOT_SNTP_DEFAULT_LI                      0
// version 3, for ipv4 only
#define TC_IOT_SNTP_DEFAULT_VN                      3
// client mode
#define TC_IOT_SNTP_DEFAULT_MODE                    3
// unspecified or unavailable
#define TC_IOT_SNTP_DEFAULT_STRATUM                 0
// minimal poll interval
#define TC_IOT_SNTP_DEFAULT_POLL                    4
// minimal precision
#define TC_IOT_SNTP_DEFAULT_PREC                   -6


typedef struct _tc_iot_sntp_packet
{
    uint8_t li_vn_mode;      // li, vn, and mode.
    uint8_t stratum;         // Stratum level of the local clock.
    uint8_t poll;            // Maximum interval between successive messages.
    uint8_t precision;       // Precision of the local clock.

    uint32_t root_delay;      // Total round trip delay time.
    uint32_t root_dispersion; // Max error aloud from primary clock source.
    uint32_t ref_id;          // Reference clock identifier.

    uint32_t ref_time_s;        // Reference time-stamp seconds.
    uint32_t ref_time_f;        // Reference time-stamp fraction of a second.

    uint32_t orig_time_s;       // Originate time-stamp seconds.
    uint32_t orig_time_f;       // Originate time-stamp fraction of a second.

    uint32_t rx_time_s;         // Received time-stamp seconds.
    uint32_t rx_time_f;         // Received time-stamp fraction of a second.

    uint32_t tx_time_s;         // The most important field the client cares about. Transmit time-stamp seconds.
    uint32_t tx_time_f;         // Transmit time-stamp fraction of a second.

} tc_iot_sntp_packet;


unsigned int tc_iot_sntp_get_timestamp(const char * host, int timeout_ms);


#endif /* TC_IOT_SNTP_H */
