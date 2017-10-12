#ifndef SFCAPP_PROXY_
#define SFCAPP_PROXY_

#include <rte_hash.h>

#define PROXY_TABLE_SZ 1024

/* static rte_hash* proxy_flow_header_lkp_table*/
/* key = ipv4_5tuple ; value = ptr to packet */

/* static rte_hash* proxy_sf_address_lkp_table */
/* key = sfid (uint16_t) ; value = ether_addr */

int proxy_init(char** cfg_filename);

/* static proxy_parse_config_file(char** cfg_filename); */

static __attribute__((noreturn)) void 
proxy_main_loop(void);

/* static int proxy_encap_nsh(rte_mbuf* pkt_mbuf); */

/* static int proxy_decap_nsh(rte_mbuf* pkt_mbuf); */


#endif