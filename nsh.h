#ifndef NSH_H_
#define NSH_H_

#include <stdint.h>

#define ETHER_TYPE_NSH 0x894F /* extension to rte_ether.h ether types */

#define NSH_BASE_HEADER_LEN         4
#define NSH_SPI_LEN                 3
#define NSH_SERVICE_PATH_ID_LEN     3
#define NSH_SERVICE_INDEX_LEN       1

#define NSH_OAM_PACKET              0x2000
#define NSH_TTL_MASK                0x0FC0
#define NSH_TTL_DEFAULT             0x0FC0

#define NSH_LENGTH_MD_TYPE_1        0x0006
#define NSH_BASE_LENGHT_MD_TYPE_2   0x0002

#define NSH_MD_TYPE_MASK    0x00000F00
#define NSH_MD_TYPE_0       0x00
#define NSH_MD_TYPE_1       0x01
#define NSH_MD_TYPE_2       0x02
#define NSH_MD_TYPE_TEST    0x0F

#define NSH_SPI_MASK        0xFFFFFF00
#define NSH_SI_MASK         0x000000FF

#define NSH_NEXT_PROTO_IPV4  0x01
#define NSH_NEXT_PROTO_IPV6  0x02
#define NSH_NEXT_PROTO_ETHER 0x03
#define NSH_NEXT_PROTO_NSH   0x04
#define NSH_NEXT_PROTO_MPLS  0x05
#define NSH_NEXT_PROTO_EXP1  0xFE
#define NSH_NEXT_PROTO_EXP2  0xFF

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
void nsh_encap(struct rte_mbuf* mbuf, struct nsh_hdr *nsh_info);

/* Decapsulates the packet in pkt_mbuf, removing the NSH
 * header.
 */
void nsh_decap(struct rte_mbuf* pkt_mbuf);

/* Decrements the value of SI in a NSH encapsulated packet.
 * Returns -1 in case o failure.
 */
int nsh_dec_si(struct rte_mbuf* pkt_mbuf);


/* Initializes the NSH header with default values */
int nsh_init_header(struct nsh_hdr *nsh_header);

/* Copies the nsh header info contained in mbuf to nsh_hdr.
 * Returns -1 in case of failure.
 */ 
int nsh_get_header(struct rte_mbuf *mbuf, struct nsh_hdr *nsh_info);

#endif
