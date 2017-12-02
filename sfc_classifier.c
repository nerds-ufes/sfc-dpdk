#include <stdio.h>
#include <stdlib.h>

#include <rte_ethdev.h>
#include <rte_ether.h>
#include <rte_hash.h>
#include <rte_jhash.h>

#include "sfc_classifier.h"
#include "common.h"
#include "nsh.h"

#define BURST_TX_DRAIN_US 100

extern struct sfcapp_config sfcapp_cfg;
extern long int n_rx, n_tx;

static struct rte_hash* classifier_flow_path_lkp_table;
/* key = ipv4_5tuple ; value = nsh_spi (3 Bytes)*/

//static rte_hash* classifier_sfp_lkp_table;
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

/*static int classifier_init_sfp_table(void){

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
}*/

void classifier_add_flow_class_entry(struct ipv4_5tuple *tuple, uint32_t sfp){
    int ret;
    struct ipv4_5tuple local_tuple;
    memcpy(&local_tuple,tuple,sizeof(struct ipv4_5tuple));

    sfp = (sfp<<8) | 0xFF;

    ret = rte_hash_add_key_data(classifier_flow_path_lkp_table,&local_tuple, 
        (void *) ((uint64_t) sfp));
    SFCAPP_CHECK_FAIL_LT(ret,0,"Failed to add entry to classifier table.\n");

    printf("Added ");
    common_print_ipv4_5tuple(&local_tuple);
    printf(" -> %" PRIx32 " to classifier flow table\n",sfp);
        
    /*uint32_t it = 0;
    struct ipv4_5tuple *key;
    uint64_t data;
    ret = 1;
    printf("Printing contents of classifier_flow_path_lkp_table:\n");
    while(ret >= 0){
        ret = rte_hash_iterate(classifier_flow_path_lkp_table,(const void**) &key , (void **) &data,&it);
        
        if(ret >= 0){
            common_print_ipv4_5tuple(key); printf(" -> ");
            printf("%" PRIx32 "\n",(uint32_t) data);
        }

    }*/
}

int classifier_setup(void){

    int ret;
    ret = classifier_init_flow_path_table();
    SFCAPP_CHECK_FAIL_LT(ret,0,"Failed to initialize Classifier table\n");

    sfcapp_cfg.main_loop = classifier_main_loop;

    return 0;
}

static void classifier_handle_pkts(struct rte_mbuf **mbufs, uint16_t nb_pkts, uint64_t *drop_mask){
    uint16_t i;
    uint64_t path_info;
    struct ipv4_5tuple tuple;
    struct nsh_hdr nsh_header;
    int lkp,ret;

    *drop_mask = 0;

    for(i = 0 ; i < nb_pkts ; i++){
        
        /* Get 5-tuple */
        ret = common_ipv4_get_5tuple(mbufs[i],&tuple,0);
        COND_MARK_DROP(ret,drop_mask);
    
        /* Get matching SFP from table */
        lkp = rte_hash_lookup_data(classifier_flow_path_lkp_table,&tuple,(void**) &path_info);
        
        //COND_MARK_DROP(lkp,drop_mask);

        //common_print_ipv4_5tuple(&tuple); printf("\n");

        if(lkp >= 0){ /* Has entry in table */

            //printf("Match!\n");

            /* Encapsulate with VXLAN */
            common_vxlan_encap(mbufs[i]);
            
            nsh_init_header(&nsh_header);
            nsh_header.serv_path = (uint32_t) path_info;

            /* Encapsulate packet */
            nsh_encap(mbufs[i],&nsh_header);
            
            common_mac_update(mbufs[i],&sfcapp_cfg.port2_mac,&sfcapp_cfg.sff_addr);
        }

        /* No matching SFP, then just give back to network
         * without modification. 
         */

        sfcapp_cfg.rx_pkts++;

        //common_dump_pkt(mbufs[i],"\n=== Output packet ===\n");
    }
}


/* Receive non-NSH packets
 * Classify packet
 * Encapsulate
 * Send to forwarder
 */
__attribute__((noreturn)) void classifier_main_loop(void){
    uint16_t nb_rx;
    struct rte_mbuf *rx_pkts[BURST_SIZE];
    uint64_t drop_mask;

    for(;;){

        drop_mask = 0;

        common_flush_tx_buffers();
        
        nb_rx = rte_eth_rx_burst(sfcapp_cfg.port1,0,rx_pkts,
                    BURST_SIZE);

        /* Classify and encapsulate */
        if(likely(nb_rx > 0)){  
            classifier_handle_pkts(rx_pkts,nb_rx,&drop_mask);        
            sfcapp_cfg.tx_pkts += send_pkts(rx_pkts,sfcapp_cfg.port2,0,sfcapp_cfg.tx_buffer2,nb_rx,drop_mask); 
        }

    }
}