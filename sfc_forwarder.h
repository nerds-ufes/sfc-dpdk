#ifndef SFCAPP_FORWARDER_
#define SFCAPP_FORWARDER_

#include <rte_hash.h>
#include "common.h"

#define FORWARDER_TABLE_SZ 1024

/* static struct rte_hash* forwarder_next_sf_lkp_table*/
/* key = spi-si ; value = next sf_id (uint16_t) */

/* static struct rte_hash* forwarder_next_sf_address_table */
/* key = sf_id (uint16_t) ; value = protocol (1B) + address (6B) */

void forwarder_add_sph_entry(uint32_t sph, uint16_t sfid);

void forwarder_add_sf_address_entry(uint16_t sfid, struct ether_addr *eth_addr);

int forwarder_setup(void);

/* static forwarder_parse_config_file(char** sections, int nb_sections); */

__attribute__((noreturn)) void forwarder_main_loop(void);


#endif