#ifndef SFCAPP_PROXY_
#define SFCAPP_PROXY_

#include <rte_hash.h>

#define PROXY_MAX_FLOWS 1024
#define PROXY_MAX_FUNCTIONS 64
#define PROXY_CFG_MAX_ENTRIES 2

void proxy_add_sph_entry(uint32_t sph, uint16_t sfid);

void proxy_add_sf_address_entry(uint16_t sfid, struct ether_addr *eth_addr);

void proxy_parse_config_file(struct rte_cfgfile *cfgfile, char** sections, int nb_sections);

int proxy_setup(void);

void proxy_main_loop(void);

#endif