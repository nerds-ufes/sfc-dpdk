#ifndef SFCAPP_CLASSIFIER_
#define SFCAPP_CLASSIFIER_

#include <rte_hash.h>

#define CLASSIFIER_TABLE_SZ 1024
#define CLASSIFIER_MAX_FLOWS 1024
#define CLASSIFIER_SFP_MAX_ENTRIES 64

/* static rte_hash* classifier_flow_path_lkp_table*/
/* key = ipv4_5tuple ; value = ns`h_spi */

int classifier_setup(void);

/* classifier_parse_config_file(char** sections, int nb_sections); */

__attribute__((noreturn)) void 
classifier_main_loop(void);


#endif