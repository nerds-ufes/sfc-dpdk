#ifndef SFCAPP_FORWARDER_
#define SFCAPP_FORWARDER_

#include <rte_hash.h>

#define FORWARDER_TABLE_SZ 1024

/* static rte_hash* forwarder_next_func_lkp_table*/
/* key = spi-si ; value = next sf_id (uint16_t) */

/* static rte_hash* forwarder_next_hop_lkp_table */
/* key = sf_id (uint16_t) ; value = chain length (uint8_t) */

int forwarder_init(char** cfg_filename);

/* static forwarder_parse_config_file(char** cfg_filename); */

static __attribute__((noreturn)) void 
forwarder_main_loop(void);


#endif