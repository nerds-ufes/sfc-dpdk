#ifndef NSH_H_
#define NSH_H_

#include <stdint.h>

#define ETHER_TYPE_NSH 0x894F /* extension to rte_ether.h ether types */

#define NSH_BASE_HEADER_LEN 4
#define NSH_SERVICE_PATH_HEADER_LEN 4
//#define NSH_CONTEXT_HEADER_LEN ?

#define NSH_SERVICE_PATH_ID_LEN 3
#define NSH_SERVICE_INDEX_LEN 1

struct nsh_hdr {
    uint16_t basic_info; /* Ver, OAM bit, Unused and TTL */
    uint8_t md_type;
    uint8_t next_proto;
    uint32_t serv_path;
    uint32_t *context; /* What type should I use for this? */
} __attribute__((__packed__));

#endif