#ifndef SFCAPP_CLASSIFIER_
#define SFCAPP_CLASSIFIER_

#include <rte_hash.h>
#include "common.h"

#define CLASSIFIER_TABLE_SZ 1024
#define CLASSIFIER_MAX_FLOWS 1024
#define CLASSIFIER_SFP_MAX_ENTRIES 64

void classifier_add_flow_class_entry(struct ipv4_5tuple *tuple, uint32_t sfp);

int classifier_setup(void);

__attribute__((noreturn)) void 
classifier_main_loop(void);


#endif