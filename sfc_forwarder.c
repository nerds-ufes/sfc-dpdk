#include <stdio.h>
#include <stdlib.h>

#include <rte_ethdev.h>
#include <rte_ether.h>
#include <rte_hash.h>
#include <rte_jhash.h>

#include "sfc_forwarder.h"
#include "common.h"
#include "nsh.h"

extern struct sfcapp_config sfcapp_cfg;

static struct rte_hash *forwarder_next_sf_lkp_table;
/* key = spi-si ; value = next sf_id (uint16_t) */

static struct rte_hash *forwarder_next_sf_address_lkp_table;
/* key = sf_id (uint16_t) ; value = mac (48b in 64b) (uint64_t) */

static int forwarder_init_next_sf_table(void){

    const struct rte_hash_parameters hash_params = {
        .name = "forwarder_next_sf",
        .entries = FORWARDER_TABLE_SZ,
        .reserved = 0,
        .key_len = sizeof(uint32_t), /* <SPI,SI> */
        .hash_func = rte_jhash,
        .hash_func_init_val = 0,
        .socket_id = rte_socket_id()
    };

    forwarder_next_sf_lkp_table = rte_hash_create(&hash_params);

    if(forwarder_next_sf_lkp_table == NULL)
        return -1;
    
    return 0;
}

static int forwarder_init_sf_addr_table(void){

    const struct rte_hash_parameters hash_params = {
        .name = "forwarder_sf_addr",
        .entries = FORWARDER_TABLE_SZ,
        .reserved = 0,
        .key_len = sizeof(uint16_t), /* SFID */
        .hash_func = rte_jhash,
        .hash_func_init_val = 0,
        .socket_id = rte_socket_id()
    };

    forwarder_next_sf_address_lkp_table = rte_hash_create(&hash_params);

    if(forwarder_next_sf_address_lkp_table == NULL)
        return -1;
    
    return 0;
}

void forwarder_add_sph_entry(uint32_t sph, uint16_t sfid){
    int ret;

    ret = rte_hash_add_key_data(forwarder_next_sf_lkp_table,&sph, 
        (void *) ((uint64_t) sfid) );
    SFCAPP_CHECK_FAIL_LT(ret,0,"Failed to add stub entry to Forwarder table.\n");

    printf("Added <sph=%" PRIx32 ",sfid=%" PRIx16 ">"
            " to forwarder next sf table.\n",sph,sfid);
}

void forwarder_add_sf_address_entry(uint16_t sfid, struct ether_addr *sfmac){
    int ret;

    ret = rte_hash_add_key_data(forwarder_next_sf_address_lkp_table,&sfid, 
        (void *) common_mac_to_64(sfmac));
    SFCAPP_CHECK_FAIL_LT(ret,0,"Failed to add SF entry to forwarder table.\n");

    char buf[ETHER_ADDR_FMT_SIZE + 1];
    ether_format_addr(buf,ETHER_ADDR_FMT_SIZE,sfmac);
    printf("Added <sfid=%" PRIx16 ",mac=%s> to forwarder" 
        " SF-address table.\n",sfid,buf);
}

int forwarder_setup(void){
    int ret;

    ret = forwarder_init_next_sf_table();
    SFCAPP_CHECK_FAIL_LT(ret,0,"Failed to initialize Forwarder Next-Func table.\n");

    ret = forwarder_init_sf_addr_table();
    SFCAPP_CHECK_FAIL_LT(ret,0,"Failed to initialize Forwarder SF Address table.\n");

    sfcapp_cfg.main_loop = forwarder_main_loop;
    
    return 0;
}


static inline void forwarder_handle_pkts(struct rte_mbuf **mbufs, uint16_t nb_pkts,
uint64_t *drop_mask){
    int lkp;
    uint16_t i;
    uint64_t data;
    struct nsh_hdr nsh_header;
    uint16_t sfid;
    struct ether_addr sf_addr;

    for(i = 0 ; i < nb_pkts ; i++){

        /* Check if this packet is for me! If not, drop*/
        lkp = common_check_destination(mbufs[i],&sfcapp_cfg.port1_mac);
        if(lkp != 0){
            *drop_mask |= 1<<i; 
            continue;
        }

        nsh_get_header(mbufs[i],&nsh_header);

        /* Match SFP to SF in table */
        lkp = rte_hash_lookup_data(forwarder_next_sf_lkp_table,
                (void*) &nsh_header.serv_path,
                (void **) &data);
        COND_MARK_DROP(lkp,drop_mask);

        sfid = (uint16_t) data;
       
        if(sfid == 0){  /* End of chain */
            nsh_decap(mbufs[i]);

            /* Remove VXLAN encap! */
            rte_pktmbuf_adj(mbufs[i],
                sizeof(struct ether_hdr) +
                sizeof(struct ipv4_hdr) +
                sizeof(struct udp_hdr) +
                sizeof(struct vxlan_hdr));
        }else{
            /* Match SFID to address in table */
            lkp = rte_hash_lookup_data(forwarder_next_sf_address_lkp_table,
                    (void*) &sfid,
                    (void**) &data);
            COND_MARK_DROP(lkp,drop_mask);
            /* Update MACs */
            common_64_to_mac(data,&sf_addr);
            common_mac_update(mbufs[i],&sfcapp_cfg.port2_mac,&sf_addr);
        }
        
        sfcapp_cfg.rx_pkts++;
    }

}
__attribute__((noreturn)) void forwarder_main_loop(void){

    uint16_t nb_rx;
    struct rte_mbuf *rx_pkts[BURST_SIZE];
    uint64_t drop_mask;
    
    for(;;){
        drop_mask = 0;

        common_flush_tx_buffers();

        nb_rx = rte_eth_rx_burst(sfcapp_cfg.port1,0,rx_pkts,
                    BURST_SIZE);

        if(likely(nb_rx > 0)){
            forwarder_handle_pkts(rx_pkts,nb_rx,&drop_mask);
            /* Forwarder uses the same port for rx and tx */
            sfcapp_cfg.tx_pkts += send_pkts(rx_pkts,sfcapp_cfg.port2,0,sfcapp_cfg.tx_buffer2,nb_rx,drop_mask);
        }
    }
}