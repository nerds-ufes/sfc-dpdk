#include <stdio.h>
#include <stdlib.h>

#include <rte_ethdev.h>
#include <rte_ether.h>
#include <rte_hash.h>

#include "sfc_classifier.h"

static rte_hash* classifier_flow_path_lkp_table;
/* key = ipv4_5tuple ; value = nsh_spi (3 Bytes)*/

static rte_hash* classifier_sfp_lkp_table;
/* key = spi (nsh_spi) ; value = initial SI (uint8_t) */

static int classifier_init_flow_path_table(void){

    const struct rte_hash_parameters hash_params = {
        .name = "classifier_flow_path",
        .entries = CLASSIFIER_MAX_FLOWS,
        .reserved = 0,
        .key_len = sizeof(struct ipv4_5tuple),
        .hash_func = rte_jhash,
        .hash_func_init_val = 0,
        .socket_id = rte_socket_id()
    };

    classifier_flow_path_lkp_table = rte_hash_create(&hash_params);

    if(classifier_flow_path_lkp_table == NULL)
        return -1;
    
    return 0;
}

static int classifier_init_sfp_table(void){

    const struct rte_hash_parameters hash_params = {
        .name = "classifier_sfp",
        .entries = CLASSIFIER_MAX_FLOWS,
        .reserved = 0,
        .key_len = sizeof(uint32_t),
        .hash_func = rte_jhash,
        .hash_func_init_val = 0,
        .socket_id = rte_socket_id()
    };

    classifier_sfp_lkp_table = rte_hash_create(&hash_params);

    if(classifier_sfp_lkp_table == NULL)
        return -1;
    
    return 0;
}

int classifier_setup(char* cfg_filename){

    /* Init hash tables
     * Parse config file
     * 
     */ 
}

static void classifier_handle_pkts(struct rte_mbuf **mbufs, uint16_t nb_pkts){
    uint16_t i;
    uint32_t path_info;
    struct ipv4_5tuple tuple;
    uint8_t initial_si;
    struct nsh_hdr nsh_header;

    if(likely(nb_pkts > 0)){

        for(i = 0 ; i < nb_pkts ; i++){

            /* Get 5-tuple */
            common_ipv4_get_5tuple(mbufs[i],&tuple);
        
            /* Get matching SFP from table */
            rte_hash_lookup_data(classifier_flow_path_lkp_table,&tuple,(void**) &path_info);

            /* Get initial SI for this path from table */
            rte_hash_lookup_data(classifier_sfp_lkp_table,&path_info,(void**) &initial_si);

            /* Concatenate SPI + SI to form Service Path Header*/
            path_info = (path_info << 8) | (uint32_t) initial_si;

            nsh_init_header(&nsh_header);
            nsh_header->serv_path = path_info;

            /* Encapsulate packet */
            nsh_encap(mbufs[i],&nsh_header);
            
        }
        /* - Get 5-tuple
         * - Get entry from table
         * - Encapsulate packet
         */

        


    }
}

/* Parameters: 
 * forwarder_mac
 * SFP (ID e SI inicial)
 * classification rule (5-tuple to SFP)
/* static classifier_parse_config_file(char** cfg_filename); */


/* Receive non-NSH packets
 * Classify packet
 * Encapsulate
 * Send to forwarder
 */
__attribute__((noreturn)) void classifier_main_loop(void){
    uint16_t nb_rx, nb_tx, i;
    struct rte_mbuf *rx_pkts[BURST_SIZE];

    for(;;){

        nb_rx = rte_eth_rx_burst(sfcapp_cfg.port1,0,rx_pkts);

        /* Classify and encapsulate */
        if(likely(nb_rx > 0))
            classifier_handle_pkts(rx_pkts,nb_rx);
        
        send_pkts(rx_pkts,sfcapp_cfg.port2,0,nb_rx);  
    }
}