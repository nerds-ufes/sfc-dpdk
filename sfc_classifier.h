#ifndef SFCAPP_CLASSIFIER_
#define SFCAPP_CLASSIFIER_

#include <rte_hash.h>

#define CLASSIFIER_TABLE_SZ 1024

/* static rte_hash* classifier_flow_path_lkp_table*/
/* key = ipv4_5tuple ; value = nsh_spi */

/* static rte_hash* classifier_sfp_lkp_table */
/* key = spi (nsh_spi) ; value = chain length (uint8_t) */

int classifier_setup(char* cfg_filename);

/* static classifier_parse_config_file(char** cfg_filename); */

__attribute__((noreturn)) void 
classifier_main_loop(void);


#endif