#ifndef NSH_H_
#define NSH_H_

#include <stdint.h>

#define ETHER_TYPE_NSH 0x894F /* extension to rte_ether.h ether types */

#define NSH_BASE_HEADER_LEN 4
#define NSH_SPI_LEN 3
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

struct nsh_spi {
    uint8_t spi_bytes[NSH_SPI_LEN];
} __attribute__((__packed__));


/* Encapsulates the packet in pkt_mbuf in NSH header with
 * NSH parameters given by service_header arg.
 */
void nsh_encap(rte_mbuf* pkt_mbuf, uint32_t service_header);

/* Desencapsulates the packet in pkt_mbuf, removing the NSH
 * header and returning a pointer to the removed header.
 */
rte_mbuf* nsh_decap(rte_mbuf* pkt_mbuf);

void nsh_dec_si(rte_mbuf* pkt_mbuf);
#endif