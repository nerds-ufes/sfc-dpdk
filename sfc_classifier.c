#include <stdio.h>
#include <stdlib.h>

#include <rte_ethdev.h>
#include <rte_ether.h>
#include <rte_hash.h>
#include <rte_jhash.h>

#include "sfc_classifier.h"
#include "common.h"
#include "nsh.h"

extern struct sfcapp_config sfcapp_cfg;

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

int classifier_setup(void){

    int ret, lkp;
    struct ipv4_5tuple tuple;
    uint64_t nsh_header_64;

    ret = classifier_init_flow_path_table();
    SFCAPP_CHECK_FAIL_LT(ret,0,"Failed to initialize Classifier table");

    tuple.src_ip    = 0x01020304;
    tuple.dst_ip    = 0x04030201;
    tuple.src_port  = 0x3039;
    tuple.dst_port  = 0xd431;
    tuple.proto     = 0x6;
    nsh_header_64 = 0 | (0x000001FF);
    lkp = rte_hash_add_key_data(classifier_flow_path_lkp_table,
                &tuple, (void *) nsh_header_64);
    if(lkp >= 0)
        printf("Successfully added flow to table! Mapped to serv_path: %08" PRIx64 "\n",nsh_header_64);

    sfcapp_cfg.main_loop = classifier_main_loop;

    return 0;
}

static void classifier_handle_pkts(struct rte_mbuf **mbufs, uint16_t nb_pkts){
    uint16_t i;
    uint64_t path_info;
    struct ipv4_5tuple tuple;
    struct nsh_hdr nsh_header;
    uint16_t offset;

    offset = sizeof(struct ether_hdr) + sizeof(struct ipv4_hdr) + 
    sizeof(struct udp_hdr) + sizeof(struct vxlan_hdr);

    for(i = 0 ; i < nb_pkts ; i++){

        //common_dump_pkt(mbufs[i],"\n=== Input packet ===\n");

        /* Get 5-tuple */
        common_ipv4_get_5tuple(mbufs[i],&tuple,offset);
    
        /* Get matching SFP from table */
        rte_hash_lookup_data(classifier_flow_path_lkp_table,&tuple,(void**) &path_info);

        nsh_init_header(&nsh_header);
        nsh_header.serv_path = (uint32_t) path_info;
        //printf("Adding the following serV_path to the packet: %" PRIx32 "\n",nsh_header.serv_path);

        /* Encapsulate packet */
        nsh_encap(mbufs[i],&nsh_header);
        
        common_mac_update(mbufs[i],&sfcapp_cfg.port2_mac,&sfcapp_cfg.sff_addr);

        //common_dump_pkt(mbufs[i],"\n=== Output packet ===\n");
    }
}

/* Parameters: 
 * forwarder_mac
 * SFP (ID e SI inicial)
 * classification rule (5-tuple to SFP)*/
/* static classifier_parse_config_file(char** cfg_filename); */


/* Receive non-NSH packets
 * Classify packet
 * Encapsulate
 * Send to forwarder
 */
__attribute__((noreturn)) void classifier_main_loop(void){
    uint16_t nb_rx;
    struct rte_mbuf *rx_pkts[BURST_SIZE];

    for(;;){

        nb_rx = rte_eth_rx_burst(sfcapp_cfg.port1,0,rx_pkts,
                    BURST_SIZE);

        /* Classify and encapsulate */
        if(likely(nb_rx > 0))
            classifier_handle_pkts(rx_pkts,nb_rx);
        
        send_pkts(rx_pkts,sfcapp_cfg.port2,0,nb_rx);  
    }
}