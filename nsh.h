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
} __attribute__((__packed__));

struct nsh_spi {
    uint8_t spi_bytes[NSH_SPI_LEN];
} __attribute__((__packed__));


/* Encapsulates the packet in pkt_mbuf in NSH header with
 * NSH parameters given by nsh_hdr. This app considers that
 * NSH packets don't contain metadata
 */
void nsh_encap(rte_mbuf* pkt_mbuf, uint64_t nsh_hdr);

/* Decapsulates the packet in pkt_mbuf, removing the NSH
 * header.
 */
void nsh_decap(struct rte_mbuf* pkt_mbuf);

/* Decrements the value of SI in a NSH encapsulated packet.
 * Returns -1 in case o failure.
 */
int nsh_dec_si(struct rte_mbuf* pkt_mbuf);

/* Copies the nsh header info contained in mbuf to nsh_hdr.
 * Returns -1 in case of failure.
 */ 
int nsh_get_header(struct rte_mbuf *mbuf, uint64_t *nsh_hdr);

#endif