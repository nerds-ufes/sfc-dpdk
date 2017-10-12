#include <stdlib.h>

#include <rte_hash.h>

#include "sfc_proxy.h"
#include "sfc_common.h"

struct sfcapp_proxy_table_key {
    struct ipv4_5tuple;     /* 5-tuple from the incoming packet */
    uint32_t serv_path;     /* SPI and SI from NSH header*/
};

struct rte_hash *sfcapp_proxy_lkp_table;

int 
proxy_init(){
    struct rte_hash_parameters hash_params = {
        .name = "proxy_table",
        .entries = PROXY_TABLE_SZ,
        .reserved = 0;
        .key_len = sizeof(struct sfcapp_proxy_table_key);
        .rte_hash_function = rte_jhash;
        .hash_func_init_val = 0;
        .socket_id = rte_socket_id();
    };

    sfcapp_proxy_lkp_table = rte_hash_create(hash_params);
    if(sfcapp_proxy_lkp_table == NULL)
        return -1;

    return 0;
}

int
proxy_main_loop(__attribute__((unused)) void *nothing){
}


