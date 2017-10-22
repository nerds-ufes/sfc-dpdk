#ifndef SFCAPP_PROXY_
#define SFCAPP_PROXY_

#include <rte_hash.h>

/* static rte_hash* proxy_flow_header_lkp_table*/
/* key = ipv4_5tuple ; value = ptr to packet */

/* static rte_hash* proxy_sf_address_lkp_table */
/* key = sfid ; value = ether_addr */

/* static int proxy_parse_config_file(char** cfg_filename); */

int proxy_setup(char *cfg_filename);


void proxy_main_loop(void);

#endif