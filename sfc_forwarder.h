#ifndef SFCAPP_FORWARDER_
#define SFCAPP_FORWARDER_

#include <rte_hash.h>
#include "common.h"

#define FORWARDER_TABLE_SZ 1024

void forwarder_add_sph_entry(uint32_t sph, uint16_t sfid);

void forwarder_add_sf_address_entry(uint16_t sfid, struct ether_addr *eth_addr);

int forwarder_setup(void);

__attribute__((noreturn)) void forwarder_main_loop(void);


#endif